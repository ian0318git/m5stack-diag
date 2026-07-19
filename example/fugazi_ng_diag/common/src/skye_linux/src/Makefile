# $Id: Makefile,v 1.2 2015/05/25 03:59:15 steja Exp $ 
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/Makefile,v $ 
#------------------------------------------------------------------
#
# Makefile - Makefile to build Skye Diagnostic.
#
# Jul 18, palin2 created for Skye.
#
# Copyright (c) 2014-2015 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#

###################################################################
#
# NOTE: When new files are added please sort it properly.
#
###################################################################

#
# NOTE: Need to build on a Linux box.
#

#
# Crash us if /bin/make is used:
#

ifeq ($(origin USER),undefined)
USER=Bozo
endif

default: all

#
# Tool Chain
#
CC		= $(TILERA_ROOT)/bin/tile-gcc
AS		= as
AR		= ar
LD		= ld
MAKE		= make 
NM		= nm
OBJCOPY		= objcopy
OBJDUMP		= objdump
SIZE		= size
STRIP		= strip

.PHONY: all clean clobber depend force-tools install

SHELL		= /bin/sh

#
# The platform specific source directory
#
SRCDIR		= .
IROOT		= ../
INCDIR		= $(IROOT)/include
KLMDIR		= $(IROOT)/klm

#
# Include paths for compiler
#
INC	= \
	-I$(INCDIR) \
	-I$(SRCDIR)

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
#
DEFS	= \
	-DLINUX_APP \
	-DL_ENDIAN \
	-DSKYE_ENHANCED_ERR_MSG

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

OPT	        = $(INC)
CFLAGS      = -std=gnu99 -Wall -Werror -g -fPIC $(OPT) ${CPPOPTS}
CPPOPTS     = -fPIC -Wall -Werror $(INC) ${DEFS} ${BCM_DEFS}
CCOPTS      = -g -fsigned-char -pipe  -Wa, 
ASOPTS      = -fsigned-char -float 
ASFLAGS     = $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}
LDFLAGS     = -ltmc -lpthread -lgxpci -lrt -lgxio

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= skye_lnx
NETELF		= $(NETTARGET).elf

#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#
TARGET_OBJS = \
	linux_error.o \
	menu.o \
	skye_test.o \
	skye_util.o \
	skye_main.o \
	linux_memory_tester.o \
	tilera_memtest.o \
	linux_memory_tester_utils.o \
	common_utils.o \
	query.o \
	memops.o \
	linux_mem.o \
	linux_api.o \
	testmem.o \
	diag_common_lib.o \
	diag_common_drv.o \
	diag_tlk10232_test.o \
	diag_tlk10232_lib.o \
	diag_backplane_xaui_test.o \
	diag_mv1514_test.o \
	skye_eth.o \
	skye_ext_lpbk.o \
	skye_xaui.o \
	skye_i2c_api.o \
	skye_dimm.o \
	skye_i2c_mux.o \
	skye_i2c.o \
	skye_led.o \
	skye_usb_test.o \
	szalinski_diag.o \
	use_power.o \
	pcie_test.o \
	skye_gpio.o \
	szalinski_fw.o \
	skye_thermal.o \
	skye_current.o \
	skye_clock_margin.o \
	skye_clkbuf.o \
	skye_spi_test.o \
	pwr_seq_diag.o \
	ping_test.o

TARGET_BASE = \
	$(SRCDIR)/linux_error.c \
	$(SRCDIR)/menu.c \
	$(SRCDIR)/skye_test.c \
	$(SRCDIR)/skye_util.c \
	$(SRCDIR)/skye_main.c \
	$(SRCDIR)/linux_memory_tester.c \
	$(SRCDIR)/tilera_memtest.c \
	$(SRCDIR)/linux_memory_tester_utils.c \
	$(SRCDIR)/common_utils.c \
	$(SRCDIR)/query.c \
	$(SRCDIR)/memops.c \
	$(SRCDIR)/linux_mem.c \
	$(SRCDIR)/linux_api.c \
	$(SRCDIR)/testmem.c \
	$(SRCDIR)/diag_common_lib.c \
	$(SRCDIR)/diag_common_drv.c \
	$(SRCDIR)/diag_tlk10232_test.c \
	$(SRCDIR)/diag_tlk10232_lib.c \
	$(SRCDIR)/diag_backplane_xaui_test.c \
	$(SRCDIR)/diag_mv1514_test.c \
	$(SRCDIR)/skye_eth.c \
	$(SRCDIR)/skye_ext_lpbk.c \
	$(SRCDIR)/skye_xaui.c \
	$(SRCDIR)/skye_i2c_api.c \
	$(SRCDIR)/skye_dimm.c \
	$(SRCDIR)/skye_i2c_mux.c \
	$(SRCDIR)/skye_i2c.c \
	$(SRCDIR)/skye_led.c \
	$(SRCDIR)/skye_usb_test.c \
	$(SRCDIR)/szalinski_diag.c \
	$(SRCDIR)/use_power.c \
	$(SRCDIR)/pcie_test.c \
	$(SRCDIR)/skye_gpio.c \
	$(SRCDIR)/szalinski_fw.c \
	$(SRCDIR)/skye_thermal.c \
	$(SRCDIR)/skye_current.c \
	$(SRCDIR)/skye_clock_margin.c \
	$(SRCDIR)/skye_clkbuf.c \
	$(SRCDIR)/skye_spi_test.c \
	$(SRCDIR)/pwr_seq_diag.c \
	$(SRCDIR)/ping_test.c

TARGET_HFILES = \
	$(INCDIR)/common.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/defs.h \
	$(INCDIR)/error.h \
	$(INCDIR)/menu.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_eth.h \
	$(INCDIR)/nvmonvars.h \
	$(INCDIR)/types.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/queryflags.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/testmem.h \
	$(INCDIR)/mem_mgr.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/router_if.h \
	$(INCDIR)/sgmii_defs.h \
	$(INCDIR)/diag_common_lib.h \
	$(INCDIR)/diag_common_drv.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/skye_eth.h \
	$(INCDIR)/skye_ext_lpbk.h \
	$(INCDIR)/skye_xaui.h \
	$(INCDIR)/skye_i2c.h \
	$(INCDIR)/fpga_szalinski.h \
	$(INCDIR)/skye_dimm.h \
	$(INCDIR)/skye_led.h \
	$(INCDIR)/skye_smi_lib.h \
	$(INCDIR)/diag_tlk10232_test.h \
	$(INCDIR)/diag_tlk10232_lib.h \
	$(INCDIR)/diag_backplane_xaui_test.h \
	$(INCDIR)/diag_mv1514_test.h \
	$(INCDIR)/skye_clkbuf.h \
	$(INCDIR)/pwr_seq_diag.h \
	$(INCDIR)/skye_comm_lib.h \
	$(INCDIR)/platform_fru.h

#
#########   vpath   #########
#
vpath %.c \
	$(IROOT): \
	$(SRCDIR): 

vpath %.h \
	$(INCDIR): \
	$(SRCDIR):

.SUFFIXES:
.SUFFIXES: .o .lis .S .c .s .dis

%.o : %.S
	$(CC) -c $(ASFLAGS) $<
%.o : %.c
	$(CC) -c $(CFLAGS) $<
%.lis : %.c
	lister3 CC=$(CC) AS=$(AS) $< $(CFLAGS) | grep -v '.stab' >$*.lis

%.dis : %.o
	$(OBJDUMP) -l --disassemble --source $< > $*.dis

%.i : %.c
	$(CC) -E $(CFLAGS) $< > $*.i

%.s : %.c
	$(CC) -S -fsource-asm $(CFLAGS) $< 

all: $(NETTARGET)

#
# do not delete the last line for object dump just in case
# we need to do this when debugging a problem. temp comment out
#

install: $(NETTARGET) install.sh
	sh install.sh
	touch install.sh
	$(OBJDUMP) -l --disassemble --source $(NETELF) > $(NETTARGET).dis


clean:
	rm -f *.o  banner.c \
	tools temp.* *.dat .compile ID.temp

clobber: clean
	rm -f moncode* pfsi syms*

superclean: clobber
	$(MAKE) --no-print-directory -C $(UTIL_DIR) clean
	$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc clean

force-tools:
	rm -f tools

tools:
	@$(MAKE) --no-print-directory -C $(UTIL_DIR) depend $(UTOOLS)
	@$(MAKE) --no-print-directory -C $(IROOT)/sys/obj-4k-c3640 mksum_elf
	@$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc depend $(HTOOLS)

TARGET_DEPEND = \
	Makefile \
	$(TARGET_OBJS)

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#
$(NETTARGET) : $(TARGET_HFILES) $(TARGET_DEPEND)
	@sh $(IROOT)/utils/banner.sh SKYE
	$(CC) $(CFLAGS) -c banner.c
	$(CC) banner.o $(LDFLAGS) $(CFLAGS) $(TARGET_OBJS) -o $(NETTARGET) 

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch skye_lnx.symbols

#
# The include file dependencies
#

DEPEND_LST = $(TARGET_BASE) $(TARGET_HFILES)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(TARGET_BASE) > dependancies
	@echo "Computing TAGS file for common/src"
	/router/bin/etags -t $(DEPEND_LST)
	@echo "Computing ID file for common/src"

scope:
	cscope -b $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies


# DO NOT DELETE THIS LINE -- make depend depends on it.

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.2  2015/05/25 03:59:15  steja
# Add Support Skye SM
#
# Revision 1.1.4.2  2015/04/29 11:36:31  steja
# Code check-in to skye-branch2 for ER code review
#
#-------------------------------------------------------------------------------
# Revision 1.1.2.8  2014/09/16 15:28:24  steja
# Remove the -DSKYE_TLK_POLARITY
#
# Revision 1.1.2.7  2014/08/28 02:54:26  steja
# Support Do all test for NC command
#
# Revision 1.1.2.6  2014/08/26 06:37:45  steja
# Remove Polarity for P1A BST Testing
#
# Revision 1.1.2.4  2014/08/25 06:17:27  palin2
# For Skye P1A BST rev., temporarily disabled enhanced error message support
# since it is not totally ready.
#
# Revision 1.1.2.3  2014/08/22 04:58:54  palin2
# First check-in to enhance Skye error message.
#
# Revision 1.1.2.2  2014/08/15 03:27:41  palin2
# Initial check-in to support NC command on Skye.
#
# Revision 1.1.2.1  2014/07/21 01:56:53  palin2
# Initial check-in Skye module side Diag code.
#
# $Endlog$
