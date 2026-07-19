# $Id: Makefile,v 1.5 2020/08/07 09:02:35 alicehua Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
#
# Copyright (c) 2019 by Cisco Systems, Inc.
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

default: all


TOOLS_VER	= c4.7.0-p3
TOOLS_ARCH	= x86_64

#
# Tool Chain
#

CC		= gcc.$(TOOLS_VER).$(TOOLS_ARCH)
AS		= as
AR		= ar.$(TOOLS_VER).$(TOOLS_ARCH)
LD		= ld
MAKE		= make 
NM		= nm.$(TOOLS_VER).$(TOOLS_ARCH)
OBJCOPY		= objcopy.$(TOOLS_VER).$(TOOLS_ARCH)
OBJDUMP		= objdump.$(TOOLS_VER).$(TOOLS_ARCH)
SIZE		= size.$(TOOLS_VER).$(TOOLS_ARCH)
STRIP		= strip.$(TOOLS_VER).$(TOOLS_ARCH)

.PHONY: all clean clobber depend force-tools install

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

NUTELLADIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
MRV_88E1543_SRC = $(DEV_DRV)/dev_88e1543_marvell
LTE_SWI_SRC = $(DEV_DRV)/dev_lte_swi
NXP_LM75B_SRC = $(DEV_DRV)/dev_nxp_lm75b

# TAM lib. request : ENGTAM-1223
#TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/isr9k_curie1ru/aikido
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido_v3_4_4

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(NUTELLADIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(MRV_88E1543_SRC) \
	-I$(LTE_SWI_SRC) 
	

#
# defines for compiler
#

DEFS = \
	-DNUTELLA \
	-DLINUX_APP \
    -DTFTP_CLIENT  \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DAIKIDO_ACT2 \
	-DAIKIDO_SUPPORT_AIK \
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS}
LDFLAGS         = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into NUTELLA project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJNAME = $(TAMLIB)/tam_lib_objname.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= nutellalnx

#
# Common device driver objects
#
DEV_OBJS = \
		   dev_object.o \
		   dev_nxp_lm75b.o \
		   dev_88e1543.o \
		   dev_lte_swi.o \
		   dev_lte_swi_at.o

DEV_BASE = \
		   $(DEV_DRV)/dev_object.c \
		   $(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		   $(DEV_DRV)/dev_object.c \
		   $(MRV_88E1543_SRC)/dev_88e1543.c \
		   $(LTE_SWI_SRC)/dev_lte_swi.c \
		   $(LTE_SWI_SRC)/dev_lte_swi_at.c


DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(MRV_88E1543_SRC)/dev_88e1543.h \
	$(LTE_SWI_SRC)/dev_lte_swi.h \
	$(LTE_SWI_SRC)/dev_lte_swi_at.h

#
# Common object files
#


COM_OBJS = \
    linux_main.o \
	linux_i2c.o \
    linux_error.o \
    linux_stub.o \
    linux_mem.o \
    linux_api.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    linux_block_test.o \
    linux_i2c_scan_test.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	linux_ntwk.o \
	linux_coretest.o \
	rtc.o \
	key_tlv_parser.o

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/linux_block_test.c \
    $(COMSRCDIR)/linux_i2c_scan_test.c \
    $(COMSRCDIR)/menu.c \
    $(COMSRCDIR)/common_utils.c \
    $(COMSRCDIR)/print_hex.c \
    $(COMSRCDIR)/query.c \
    $(COMSRCDIR)/linux_pcie.c \
    $(COMSRCDIR)/memops.c \
    $(COMSRCDIR)/testmem.c \
    $(COMSRCDIR)/bit_swaps.c \
    $(COMSRCDIR)/tftpclient.c \
    $(COMSRCDIR)/cookie_4_core.c \
    $(COMSRCDIR)/tam_act2_utils.c \
    $(COMSRCDIR)/linux_ntwk.c \
    $(COMSRCDIR)/linux_popen.c \
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/tam_aikido_upgrade.c \
    $(COMSRCDIR)/rtc.c \
    $(COMSRCDIR)/key_tlv_parser.c


COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
    $(INCDIR)/common_utils.h \
    $(INCDIR)/linux_usb_test.h \
    $(INCDIR)/linux_block_test.h \
    $(INCDIR)/linux_i2c_scan_test.h \
    $(INCDIR)/i2c_api.h \
    $(INCDIR)/i2c_dev.h \
    $(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/linux_ntwk.h \
	$(INCDIR)/key_tlv_parser.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h \

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files :
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	mb_tests.o \
	linux_popen.o \
	diag_boot_flash_test.o \
	diag_spi_flash_util.o \
	diag_spi_flash_lib.o \
	diag_fpga_test.o \
	diag_gephy_test.o \
	diag_gephy_lib.o \
	diag_gephy_util.o \
	diag_fpga_lib.o \
	diag_led_test.o \
	diag_led_util.o \
        diag_eth_pkt_txrx.o \
        diag_eth_pkt_txrx_api.o \
        diag_eth_pkt_txrx_utils.o \
	diag_lte_test.o \
	diag_lte_lib.o \
	diag_ddr4_lib.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	i2c_api.o \
	diag_fpga_i2c_lib.o \
	diag_usb_lib.o \
	diag_usb_util.o \
	diag_usb_test.o \
	diag_temp_snsr_lib.o \
	diag_temp_snsr_test.o \
	diag_cpu_util.o \
	platform_stub.o \
	platform_cookie.o \
	tam_act2_api_drv_support.o \
	tam_aikido_upgrade.o \
	diag_emmc_test.o \
	diag_emmc_util.o \
	dnv_gpio_lib.o \
	diag_uart_lib.o \
	dnv_eth_lib.o \
	diag_fpga_upgrade.o \
	diag_rtc_lib.o \
	diag_rtc_test.o \
	diag_rtc_util.o \
	platform_cpu.o \
	diag_reset_button.o \
	nutella_comm.o \
	full_load_util.o \
	diag_i350_test.o \
	diag_lte_util.o \
	platform_spi.o \
	diag_sfp_util.o

PLAT_BASE = \
	$(NUTELLADIR)/linux_main.c \
	$(NUTELLADIR)/diag.c \
	$(NUTELLADIR)/mb_tests.c \
	$(NUTELLADIR)/diag_boot_flash_test.c \
	$(NUTELLADIR)/diag_spi_flash_util.c \
	$(NUTELLADIR)/diag_spi_flash_lib.c \
	$(NUTELLADIR)/diag_fpga_test.c \
	$(NUTELLADIR)/diag_gephy_test.c \
	$(NUTELLADIR)/diag_gephy_lib.c \
	$(NUTELLADIR)/diag_gephy_util.c \
	$(NUTELLADIR)/diag_fpga_lib.c \
	$(NUTELLADIR)/diag_led_test.c \
	$(NUTELLADIR)/diag_led_util.c \
	$(NUTELLADIR)/diag_eth_pkt_txrx.h \
	$(NUTELLADIR)/diag_eth_pkt_txrx_api.h \
	$(NUTELLADIR)/diag_eth_pkt_txrx_utils.h \
	$(NUTELLADIR)/diag_lte_test.c \
	$(NUTELLADIR)/diag_lte_lib.c \
	$(NUTELLADIR)/diag_ddr4_lib.c \
	$(NUTELLADIR)/diag_i2c_lib.c \
	$(NUTELLADIR)/diag_i2c_test.c \
	$(NUTELLADIR)/i2c_api.c \
	$(NUTELLADIR)/diag_fpga_i2c_lib.c \
	$(NUTELLADIR)/diag_fpga_upgrade.c \
	$(NUTELLADIR)/diag_usb_lib.c \
	$(NUTELLADIR)/diag_usb_util.c \
	$(NUTELLADIR)/diag_usb_test.c \
	$(NUTELLADIR)/diag_temp_snsr_lib.c \
	$(NUTELLADIR)/diag_temp_snsr_test.c \
	$(NUTELLADIR)/diag_cpu_util.c \
	$(NUTELLADIR)/platform_stub.c \
	$(NUTELLADIR)/platform_cookie.c \
	$(NUTELLADIR)/tam_act2_api_drv_support.c \
	$(NUTELLADIR)/diag_emmc_test.c \
	$(NUTELLADIR)/diag_emmc_util.c \
	$(NUTELLADIR)/dnv_gpio_lib.c \
	$(NUTELLADIR)/diag_uart_lib.c \
	$(NUTELLADIR)/dnv_eth_lib.c \
	$(NUTELLADIR)/diag_rtc_lib.c \
	$(NUTELLADIR)/diag_rtc_test.c \
	$(NUTELLADIR)/diag_rtc_util.c \
	$(NUTELLADIR)/platform_cpu.c \
	$(NUTELLADIR)/diag_reset_button.c \
	$(NUTELLADIR)/nutella_comm.c \
	$(NUTELLADIR)/full_load_util.c \
	$(NUTELLADIR)/diag_i350_test.c \
	$(NUTELLADIR)/diag_lte_util.c \
	$(NUTELLADIR)/platform_spi.c \
	$(NUTELLADIR)/diag_sfp_util.c


PLAT_HFILES = \
    $(NUTELLADIR)/diag_common.h \
    $(NUTELLADIR)/platform_i2c.h \
    $(NUTELLADIR)/mb_tests.h \
    $(NUTELLADIR)/diag_eth_info.h \
    $(NUTELLADIR)/diag_eth_pkt_txrx.h \
    $(NUTELLADIR)/diag_eth_pkt_txrx_api.h \
    $(NUTELLADIR)/diag_eth_pkt_txrx_utils.h \
    $(NUTELLADIR)/diag_fpga.h \
    $(NUTELLADIR)/diag_ddr4_lib.h \
    $(NUTELLADIR)/diag_i2c_lib.h \
    $(NUTELLADIR)/diag_i2c_test.h \
    $(NUTELLADIR)/diag_i2c_addr.h \
    $(NUTELLADIR)/diag_boot_flash_test.h \
    $(NUTELLADIR)/diag_spi_flash_util.h \
    $(NUTELLADIR)/diag_spi_flash_lib.h \
    $(NUTELLADIR)/diag_fpga_i2c_lib.h \
    $(NUTELLADIR)/diag_storage_lib.h \
    $(NUTELLADIR)/diag_usb_lib.h \
    $(NUTELLADIR)/diag_usb_util.h \
    $(NUTELLADIR)/diag_usb_test.h \
    $(NUTELLADIR)/diag_temp_snsr_lib.h \
    $(NUTELLADIR)/diag_temp_snsr_test.h \
    $(NUTELLADIR)/diag_gephy_test.h \
    $(NUTELLADIR)/diag_gephy_lib.h \
    $(NUTELLADIR)/diag_gephy_util.h \
    $(NUTELLADIR)/diag_cpu_util.h \
    $(NUTELLADIR)/platform_stub.h \
    $(NUTELLADIR)/platform_cookie.h \
    $(NUTELLADIR)/tam_act2_api_drv_support.h \
    $(NUTELLADIR)/diag_emmc_test.h \
    $(NUTELLADIR)/diag_emmc_util.h \
    $(NUTELLADIR)/dnv_gpio_lib.h \
    $(NUTELLADIR)/diag_uart_lib.h \
    $(NUTELLADIR)/dnv_eth_lib.h \
    $(NUTELLADIR)/diag_fpga_upgrade.h \
    $(NUTELLADIR)/diag_rtc_lib.h \
    $(NUTELLADIR)/diag_rtc_test.h \
    $(NUTELLADIR)/diag_rtc_util.h \
    $(NUTELLADIR)/platform_cpu.h \
    $(NUTELLADIR)/diag_reset_button.h \
    $(NUTELLADIR)/nutella_comm.h \
    $(NUTELLADIR)/full_load_util.h \
	$(NUTELLADIR)/diag_i350_test.h \
    $(NUTELLADIR)/diag_lte_util.h \
	$(NUTELLADIR)/diag_sfp_util.c

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_TAM_OBJNAME) \

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE)
#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(NUTELLADIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(MRV_88E1543_SRC): \
	$(LTE_SWI_SRC): \
	$(TAMLIB)

vpath %.h \
	$(GE_PHY_SRC): \
	$(NUTELLADIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
    $(RTC_SRC): \
	$(NXP_LM75B_SRC): \
    $(MRV_88E1543_SRC): \
    $(LTE_SWI_SRC): \
    $(TAMLIB)

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

#
# No ROM version for informers
#

all: $(NETTARGET)

clean:
	rm -rf *.o banner.c \
	.compile ID.temp


TARGET_DEPEND = \
	Makefile \
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)
	
ENV_PRT :
	@echo "##########################################################"
	@echo "#################### Env Variables #######################"
	@echo "##########################################################"
	@echo "PATH: " $(PATH)
	@echo "NUTELLA_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "NUTELLADIR: " $(NUTELLADIR)
	@echo "KLMDIR: " $(KLMDIR)
	@echo "DEV_DRV: " $(DEV_DRV)
	@echo "INCDIR: " $(INCDIR)
	@echo "COMSRCDIR: " $(COMSRCDIR)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "##########################################################"

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : ENV_PRT $(TARGET_DEPEND) $(HFILES)
	sh $(IROOT)/utils/banner.sh NUTELLA
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS}

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols
	
#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) > dependancies
	@echo "Computing TAGS file for common/src"
	#/router/bin/etags -t $(DEPEND_LST)
	$(shell which ctags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	 -lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)
	
scope:
	cscope -b $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.5  2020/08/07 09:02:35  alicehua
# CSCvv24244: Add SFP PHY read/write utilities.
#
# Revision 1.4  2019/07/11 12:31:31  alicehua
# Collapse Nutella codes into main trunk.
#
# $Endlog$
