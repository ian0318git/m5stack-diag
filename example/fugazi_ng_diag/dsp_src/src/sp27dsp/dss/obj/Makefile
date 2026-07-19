# $Id: Makefile,v 1.3 2012/07/17 20:34:17 srane Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/obj/Makefile,v $
#------------------------------------------------------------------
# Makefile
#      Graffham project - DSP DSS Core diagnostics makefile
#
# Mar 2012, Smita Rane
#
# Copyright (c) 2012 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#

# Locate the base directory for LSI DSP tools
LSI_DIR ?= /auto/aegir-ios/tools/dsp/lsi
LIBDIR = /auto/sp-engops/diags/pld/sp26dsp
PROTODIR = $(LSI_DIR)/sc3/cgtools/2.1.3p4/host

#CGEN_VERSION := 2.4.2
CGEN_VERSION := 2.9.6
#CGEN_VERSION_LINUX := 2.4.2
CGEN_VERSION_LINUX := 2.9.6
#ARCH		:= sp2603
ARCH		:= sp2704
#COREARCH    := sc3400
#BOARD		:= evb2603
BOARD		:= evb2704
OSECK_VERSION   := 4.0.4-201103
OSECK_VERSION   := 4.0.3

#FOR TDM START
DEVICES_VERSION := 2.9.3
SC_DEVICES := $(LSI_DIR)/devices/$(DEVICES_VERSION)/sp27xx
#FOR TDM END 

KRNARCH		:= $(ARCH)
#OSECKROOT	:= $(LSI_DIR)/ose/OSEck_3.3.1
OSECKROOT := $(LSI_DIR)/ose/OSEck$(OSECK_VERSION)
SILENT		:= @
SSILENT		:= -@
BSPROOT		:= $(OSECKROOT)/$(ARCH)/bsp-$(BOARD)
OBJDIR		:= .
LIBRARY_DIR     := $(LIBDIR)


TARGET_BASE		:= dss_sp2700_sw.eld

#setup paths to cgtools for build environment
system := $(shell uname -s)
ifeq ($(system), SunOS)
HOSTARCH	:= solaris2
override SC_TOOLS_HOME := $(LSI_DIR)/sc3/cgtools/$(CGEN_VERSION)
#ARMTOOLS := $(LSI_DIR)/arm/cgtools/4.1.1/bin
CONV26X := ../../devtools/conv26x/conv26x-SunOS
endif
ifeq ($(system), Linux)
HOSTARCH	:= linux
override SC_TOOLS_HOME := $(LSI_DIR)/sc3/cgtools/$(CGEN_VERSION_LINUX).Linux
#ARMTOOLS := $(LSI_DIR)/arm/cgtools/4.1.1-linux/bin
CONV26X := ../../devtools/conv26x/conv26x-Linux
endif

export SC_TOOLS_HOME
export PATH := $(SC_TOOLS_HOME)/bin:$(PATH)
MKCONFIG	 = $(SILENT)$(OSECKROOT)/$(HOSTARCH)/bin/mkconfig_$(KRNARCH)
SCLLC_TOOLS_HOME = $(SC_TOOLS_HOME)
export LD_LIBRARY_PATH := $(SC_TOOLS_HOME)/bin:$(LD_LIBRARY_PATH)


# Tools
#
CC = scc
AS = scasm
LD = sclnk
CPP := $(wildcard /router/bin/cpp.c2.95.3-p8)
ECHO		 = $(SILENT)echo
ifeq ($(strip $(CPP)),)
CPP := cpp
endif
MKCONFIG = $(OSECKROOT)/$(HOSTARCH)/bin/mkconfig_$(KRNARCH)
RM = rm -f

CC_DEFINES := USE_AG_MG_REGS

#
# Tool flags
#
CFLAGS		 = -c
CFLAGS		+= -arch sc3400
CFLAGS          += -v -abi4 -mb -dev sp2603
CFLAGS          += -mh # huge model for DMA and ext mem support
CFLAGS		+= -r $(OBJDIR)
CFLAGS          += -Wall $(addprefix -D, $(CC_DEFINES))
CFLAGS		+= -g # To use the CDA the core example must be built with debug symbols

ASFLAGS = -arch SC3400e -oIL -g -d__ABI4__ 1 -mw_comp -w -c -omb -rela -q

LDFLAGS = -Werror -arch SC3400e -c linkscript.lcf

INCLUDES	+= -I../src
INCLUDES	+= -I../inc
INCLUDES	+= -I../../ppb/inc
#FOR TDM START 
INCLUDES	+= -I$(SC_DEVICES)/ppb/include
#FOR TDM END 

#
# Which drivers to enable in the BSP (see bspboard.h for
# additional drivers/details)
#
COREOBJS	+= sc3_rt_startup_asm.eln
COREOBJS	+= diag_main.eln
#COREOBJS	+= TDM_self_loopback.eln
COREOBJS	+= uart.eln
COREOBJS	+= debug_console.eln
COREOBJS	+= test_sanity.eln

#
#
#
VPATH += ../src:
#
# StreamWare defines
#

ifeq ($(VIDEOCONFENABLE), 1)
TARGET_CDEFS += -DVIDEO_CONF_ENABLED
endif

DEFINES         += $(TARGET_CDEFS)

DEFINES         += -DLSI_SC

#
# BSPLIBROOT specifies where to seach for the bsp lib
# default is the location of the prebuild bsp library
#
# e.g. if you have rebuild the bsp in the default OSE location
#
# make BSPLIBROOT=../../bsp-sp2603/build all
#
BSPLIBROOT	?= $(BSPROOT)/lib
LIBS		+= -L$(BSPLIBROOT)

LIBS		+= -L$(OSECKROOT)/$(ARCH)/krn-$(KRNARCH)/lib
LIBS		+= -L$(OSECKROOT)/$(ARCH)/lib
LIBS        += -L$(LIBRARY_DIR)
LIBS        += -L$(LD_LIBRARY_PATH)

LIBS		+= $(SCLLC_TOOLS_HOME)/lib3400e/rtlib_abi4_le.elb

#


OBJS		:= $(addprefix $(OBJDIR)/, $(COREOBJS))

default: $(TARGET_BASE)

all: $(TARGET_BASE)

dss_sp2700_sw.eld: $(LNKFILE) $(OBJS)
	$(ECHO) "Linking $^ => $@"
	$(LD) $(LDFLAGS) -o $@ -Map dss_sp2700_sw.map $(OBJS) $(LIBS)

$(OBJDIR)/%.eln: %.c
	$(CC) -DFILE_NAME="$(basename $(notdir $<))" $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

$(OBJDIR)/%.eln: %.asm
	$(ECHO) "Assembling "$<" => "$@
	$(AS) $(ASFLAGS) -b$@ $<

clean:
	$(ECHO) "Cleaning up..."
	$(RM) $(OBJDIR)/*.eln $(OBJDIR)/*.d $(OBJDIR)/*.sl
	$(RM) dss_sp2700_sw.eld dss_sp2700_sw.map
	$(RM) $(TARGET_BASE).c $(TARGET_BASE).rbf
superclean: clean
	$(MAKE) -C ../obj superclean

# dependancy generator
ifneq "clean" "$(findstring clean,$(MAKECMDGOALS))"
  -include $(subst .eln,.d,$(OBJS))
endif
%.d: %.c
	$(ECHO) "Compute dependencies for $<"
	$(SILENT)$(CC) $(addprefix -D, $(CC_DEFINES)) $(DEFINES) $(INCLUDES) \
	-M $@ $< -o $(basename $@).eln

# Include the static analysis makefile support
# only if running static analysis.
# If not running static analysis no impact on regular builds
ifneq ($(SES),)
#Include the SES static analysis makefile if running static analysis
include /auto/ses/repository/static_analysis/sesmake_include
endif # End of ifneq ($(SES),)


#
# $Log: Makefile,v $
# Revision 1.3  2012/07/17 20:34:17  srane
# cleanup
#
# Revision 1.2  2012/05/10 22:44:46  srane
# Add DSS sanity test for all cores.
#
# Revision 1.1  2012/04/18 18:15:24  srane
# Initial checkin
#
# 
# $Endlog$

