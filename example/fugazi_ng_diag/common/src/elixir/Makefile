# $Id: Makefile,v 1.2 2021/09/24 01:21:05 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/Makefile,v $
#------------------------------------------------------------------
#
# Makefile
#
# Copyright (c) 2019 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------

###################################################################
#
# NOTE: When new files are added please sort it properly.
#
###################################################################
#
# NOTE: Need to build on a Linux box.
#

default: all

#
# Tool Chain, Please modify in .config
#

# Using the local config
# ifeq ($(CROSS_COMPILE),)
# include .config
# export PATH :=$(CROSS_COMPILE_TOOL_PATH):$(PATH)
# endif

# Using toolchain on Cisco server
CROSS_COMPILE_TOOL_PATH = /auto/srg-diag-xcompiler/APM/ESPN/Toolchain/LE/apm-aarch64/6.0.4/bin/
CROSS_COMPILE		= $(CROSS_COMPILE_TOOL_PATH)aarch64-apm-linux-gnu-

CC				= $(CROSS_COMPILE)gcc
AS				= $(CROSS_COMPILE)as
AR				= $(CROSS_COMPILE)ar
LD				= $(CROSS_COMPILE)ld
NM				= $(CROSS_COMPILE)nm
OBJCOPY			= $(CROSS_COMPILE)objcopy
OBJDUMP			= $(CROSS_COMPILE)objdump
SIZE			= $(CROSS_COMPILE)size
STRIP			= $(CROSS_COMPILE)strip
MAKE			= make 

LDFLAGS =  -lpthread -lm 
.PHONY: all clean depend

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

PROJ_DIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
DEV_MAX31730	= $(DEV_DRV)/dev_maxim_max31730
GE_PHY_SRC	= $(DEV_DRV)/dev_n88e111x_marvell
POE_TPS2386X_SRC	= $(DEV_DRV)/dev_tps2386b
PCA9555_SRC = $(DEV_DRV)/dev_pca9555
PCA9557_SRC = $(DEV_DRV)/dev_pca9557
TMPX75_SRC  = $(DEV_DRV)/dev_tmpx75
NXP_LM75B_SRC  = $(DEV_DRV)/dev_nxp_lm75b
LTC_SRC		= $(DEV_DRV)/dev_ltc4215
TELIT_LTE_SRC   = $(DEV_DRV)/dev_lte_telit
NR_5G_TELIT_SRC = $(DEV_DRV)/dev_NR_5G_telit
PLUG_COMMON_SRC = ../plug_common
PLUG_TC_SRC     = ../plug_testcard
PLUG_LTE_SRC    = ../plug_lte
PLUG_NR_5G_SRC  = ../plug_NR_5G
PLUG_LTE_TELIT_SRC = $(PLUG_LTE_SRC)/plug_lte_telit
PLUG_NR_5G_TELIT_SRC = $(PLUG_NR_5G_SRC)/plug_NR_5G_telit
PLUG_SERIAL_SRC = ../plug_serial

#
# For USB dongle
# 

LTE_SWI_SRC = $(DEV_DRV)/dev_lte_swi
USB_DONG_DIR         = ../usb_dongle
USB_DONG_COMMON_SRC  = $(USB_DONG_DIR)/usb_dongle_common
USB_DONG_LTE_SWI_SRC = $(USB_DONG_DIR)/usb_dongle_lte_swi

#
# For AC5 switch
#

CPSS_SRC = ../cpss_common
CPSS_TOOL_LIB = /auto/sp-engops/diags/pld/cpss42_tool_lib
CPSS_TOOL_LIB_PHY_SRC = $(CPSS_TOOL_LIB)/CPSS_phy
CPSS_TOOL_LIB_PHY_HEADER = $(CPSS_TOOL_LIB)/CPSS_phy/phy_header
CPSS_TOOL_LIB_SWITCH_SRC = $(CPSS_TOOL_LIB)/compilation_root/cpss_4.2/armv8_DX
CPSS_TOOL_LIB_SWITCH_OTHER_SRC = $(CPSS_TOOL_LIB_SWITCH_SRC)/common/src

CPSS_MAD = $(CPSS_TOOL_LIB_PHY_SRC)/madDrv.o
CPSS_COMMON = $(CPSS_TOOL_LIB_SWITCH_SRC)/common/libs/COMMON.a
CPSS_PPDRV = $(CPSS_TOOL_LIB_SWITCH_SRC)/mainPpDrv/libs/MAINPPDRV.a 


##Library for pci utils  These libraary file were copy from elixir kernel
LIB_PCI_UTILS= libpci.a
LIB_Z= libz.a
LIB_Z_128= libz.so.1.2.8


MRV_88E1680_SRC = $(DEV_DRV)/dev_88e1680_marvell
MRV_98DXC25X_SRC = $(DEV_DRV)/dev_98dxc25x_marvell

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(PROJ_DIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(EEPROM_SRC) \
	-I$(RTC_SRC) \
	-I$(DEV_MAX31730) \
	-I$(LIB_TAM_ACT2_SRC) \
	-I$(COMSRCDIR) \
	-I$(GE_PHY_SRC) \
	-I$(POE_TPS2386X_SRC) \
	-I$(PCA9555_SRC) \
	-I$(PCA9557_SRC) \
	-I$(TMPX75_SRC) \
	-I$(NXP_LM75B_SRC) \
	-I$(LTC_SRC) \
	-I$(PLUG_COMMON_SRC) \
	-I$(PLUG_TC_SRC) \
	-I$(PLUG_LTE_SRC) \
	-I$(PLUG_LTE_TELIT_SRC) \
	-I$(PLUG_SERIAL_SRC) \
	-I$(TELIT_LTE_SRC) \
	-I$(PLUG_NR_5G_TELIT_SRC) \
	-I$(NR_5G_TELIT_SRC) \
	-I$(CPSS_TOOL_LIB_SWITCH_SRC) \
	-I$(MRV_88E1680_SRC) \
	-I$(MRV_98DXC25X_SRC) \
	-I$(USB_DONG_COMMON_SRC)\
	-I$(USB_DONG_LTE_SWI_SRC)\
	-I$(LTE_SWI_SRC) \

INC     += \
	-I$(CPSS_SRC) \
	-I$(CPSS_TOOL_LIB_PHY_HEADER) \
        -I$(CPSS_TOOL_LIB)/common \
        -I$(CPSS_TOOL_LIB)/common/h \
        -I$(CPSS_TOOL_LIB)/common/h/cpss \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/config \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/config/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/cos \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/cpssHwInit \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/cpssHwInit/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/diag \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/diag/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/init \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/init/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/networkIf \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/phy \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/phy/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/port \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/port/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/smi \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/systemRecovery \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/common/systemRecovery/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/driver \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/driver/interrupts \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/extDrv \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/extDrv/drivers \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/os \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/os/gtOs \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/os/gtStack \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/extServices/trace \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/generic \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/generic/events \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/generic/events/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/generic/log \
        -I$(CPSS_TOOL_LIB)/common/h/cpss/generic/version \
        -I$(CPSS_TOOL_LIB)/common/h/cpssCommon \
        -I$(CPSS_TOOL_LIB)/common/h/cpssCommon/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/log \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/log/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/config \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/config/generic \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/hardware \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/hardware/private \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/interrupts \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/interrupts/dxExMx \
        -I$(CPSS_TOOL_LIB)/common/h/cpssDriver/pp/interrupts/generic \
        -I$(CPSS_TOOL_LIB)/mainPpDrv \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxCh3 \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxCh3/policer \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxCh3/policer/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/config \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/config/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cos \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cos/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/diag \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ip/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/log \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/log/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/lpm \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/lpm/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/nst \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/nst/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/oam \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/oam/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/phy \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/policer \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/macCtrl \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PortMapping \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/hw \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/ram \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/tcam \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/protection \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/protection/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tcam \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tcam/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tmGlue \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tmGlue/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tti \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tti/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/version \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/version/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/virtualTcam \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/virtualTcam/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/bridge \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/bridge/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/config \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/config/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cos \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cos/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cpssHwInit \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cpssHwInit/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cscd \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/cscd/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/diag \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/diag/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/diag/private/serdesOpt \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/dragonite \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/dragonite/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/extMac \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/extMac/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/i2c \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/i2c/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/ip \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/ip/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/log \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/mirror \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/mirror/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/networkIf \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/networkIf/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/nst \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/nst/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/pcl \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/pcl/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/phy \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/phy/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/policer \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/policer/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/port \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/port/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/private/utils \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/smi \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/smi/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/srvCpu \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/srvCpu/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/systemRecovery \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/systemRecovery/hsu \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/systemRecovery/hsu/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/tm \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/tm/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/trunk \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/trunk/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/tunnel \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/tunnel/private \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/version \
        -I$(CPSS_TOOL_LIB)/mainPpDrv/h/cpss/generic/version/private \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/include \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/sd28firmware \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/sdAvagoFirmware \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/port/silicon/serviceCpu \
		-I$(CPSS_TOOL_LIB)/common/src/cpss/common/labServices/port/gop/src/portCtrl/h \



#
# defines for compiler
#

DEFS = \
	-DTSN \
	-DENHANCE_ERROR_MSG_RDY \
	-DLINUX_APP \
    -DTFTP_CLIENT  \
	-DAIKIDO_ACT2 \
    -DSREC2BIN \
	-DTSN_CSCvg25685_AIKIDO_TAM_WDC_ISSUE_WORKAROUND \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DELIXIR \
    -DWDC_OBJECT_NAME \

# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.
#-DNOT_GFAST 
    #-DAIKIDO_RESET 
    # -DOLD_SWI_FW
#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS}
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into project
#
#
LIB_TAM_ACT2_SRC = /auto/sp-engops/diags/pld/act2lite/ARM/Elixir/Aikido_v3_4_24
LIB_TAM_ACT2 = $(LIB_TAM_ACT2_SRC)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJ = $(LIB_TAM_ACT2_SRC)/tam_lib_objname.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= elixirlnx

#
# Common device driver objects
#
DEV_OBJS = \
	dev_object.o \
	dev_1337.o \
	dev_maxim_max31730.o \
	dev_mrvl_ge.o \
	dev_tps2386b.o \
	dev_pca9555.o \
	dev_pca9557.o \
	dev_tmpx75.o \
	dev_nxp_lm75b.o \
	dev_lte_telit.o \
	dev_lte_telit_at.o \
	dev_NR_5G_telit.o \
    dev_NR_5G_telit_at.o \
	dev_88e1680.o \
	dev_98dxc25x.o \
	dev_lte_swi.o \
	dev_lte_swi_at.o \
	dev_cpss42.o \

DEV_BASE = \
	$(DEV_DRV)/dev_object.c \
	$(RTC_SRC)/dev_1337.c \
	$(DEV_MAX31730)/dev_maxim_max31730.c \
	$(GE_PHY_SRC)/dev_mrvl_ge.c \
	$(POE_TPS2386X_SRC)/dev_tps2386b.c \
	$(PCA9555_SRC)/dev_pca9555.c \
	$(PCA9557_SRC)/dev_pca9557.c \
	$(TMPX75_SRC)/dev_tmpx75.c \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
	$(TELIT_LTE_SRC)/dev_lte_telit.c \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.c\
    $(NR_5G_TELIT_SRC)/dev_NR_5G_telit.c \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.c \
	$(MRV_98DXC25X_SRC)/dev_98dxc25x.c \
	$(MRV_88E1680_SRC)/dev_88e1680.c \
	$(LTE_SWI_SRC)/dev_lte_swi.c \
	$(LTE_SWI_SRC)/dev_lte_swi_at.c \
    $(MRV_98DXC25X_SRC)/dev_cpss42.c \

DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(RTC_SRC)/dev_1337.h \
	$(DEV_MAX31730)/dev_maxim_max31730.h \
	$(GE_PHY_SRC)/dev_mrvl_ge.h \
	$(POE_TPS2386X_SRC)/dev_tps2386b.h \
	$(PCA9555_SRC)/dev_pca9555.h \
	$(PCA9557_SRC)/dev_pca9557.h \
	$(TMPX75_SRC)/dev_tmpx75.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(TELIT_LTE_SRC)/dev_lte_telit.h \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.h\
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit.h \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.h \
	$(MRV_98DXC25X_SRC)/dev_98dxc25x.h \
	$(MRV_88E1680_SRC)/dev_88e1680.h \
	$(LTE_SWI_SRC)/dev_lte_swi.h \
	$(LTE_SWI_SRC)/dev_lte_swi_at.h \
    $(MRV_98DXC25X_SRC)/dev_cpss42.h \

#
# Pluggable Common Files
#
PLUG_COMMON_OBJS = \
    plug_common_host.o \
    plug_common_lib.o \
    plug_gpio_exp_test.o \
    plug_gpio_exp_lib.o \
    plug_slot.o \
    plug_temp_sensor_test.o \
    plug_temp_sensor_lib.o \
    plug_host_slot_modules.o \
    plug_common_host_impl.o \

PLUG_COMMON_BASE = \
    $(PLUG_COMMON_SRC)/plug_common_host.c \
    $(PLUG_COMMON_SRC)/plug_common_lib.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.c \
    $(PLUG_COMMON_SRC)/plug_slot.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.c \
    $(PROJ_DIR)/plug_host_slot_modules.c \
    $(PROJ_DIR)/plug_common_host_impl.c \

PLUG_COMMON_HFILES = \
    $(PLUG_COMMON_SRC)/plug_common_host.h \
    $(PLUG_COMMON_SRC)/plug_common_lib.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.h \
    $(PLUG_COMMON_SRC)/plug_slot.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.h \
    $(PROJ_DIR)/plug_host_slot_modules.h \
    $(PROJ_DIR)/plug_common_host_impl.h \

#
# Pluggable Testcard
#
PLUG_TC_OBJS = \
    plug_testcard_gpio_exp_lib.o \
    plug_testcard_host.o \
    plug_testcard_phy.o \
    plug_testcard_test.o \
    plug_testcard_usb_lib.o \
    plug_testcard_util.o \
    plug_testcard_host_impl.o \

PLUG_TC_BASE = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.c \
    $(PLUG_TC_SRC)/plug_testcard_host.c \
    $(PLUG_TC_SRC)/plug_testcard_phy.c \
    $(PLUG_TC_SRC)/plug_testcard_test.c \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.c \
    $(PLUG_TC_SRC)/plug_testcard_util.c \
    $(PROJ_DIR)/plug_testcard_host_impl.c \

PLUG_TC_HFILES = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_host.h \
    $(PLUG_TC_SRC)/plug_testcard_phy.h \
    $(PLUG_TC_SRC)/plug_testcard_test.h \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_util.h \
    $(PROJ_DIR)/plug_testcard_host_impl.h \

#
# Pluggable LTE
# 
PLUG_LTE_OBJS = \
    plug_lte_at.o \
    plug_lte_host.o \
    plug_lte_lib.o \
    plug_lte_test.o \
    plug_lte_util.o \
    plug_lte_host_impl.o \

PLUG_LTE_BASE = \
    $(PLUG_LTE_SRC)/plug_lte_at.c \
    $(PLUG_LTE_SRC)/plug_lte_host.c \
    $(PLUG_LTE_SRC)/plug_lte_lib.c \
    $(PLUG_LTE_SRC)/plug_lte_test.c \
    $(PLUG_LTE_SRC)/plug_lte_util.c \
    $(PROJ_DIR)/plug_lte_host_impl.c \

PLUG_LTE_HFILES = \
    $(PLUG_LTE_SRC)/plug_lte_at.h \
    $(PLUG_LTE_SRC)/plug_lte_host.h \
    $(PLUG_LTE_SRC)/plug_lte_lib.h \
    $(PLUG_LTE_SRC)/plug_lte_test.h \
    $(PLUG_LTE_SRC)/plug_lte_util.h \
    $(PROJ_DIR)/plug_lte_host_impl.h \

#
# Pluggable LTE Telit 
#
PLUG_LTE_TELIT_OBJS = \
	plug_lte_telit_host.o \
	plug_lte_telit_lib.o \
	plug_lte_telit_test.o \
	plug_lte_telit_util.o \

PLUG_LTE_TELIT_BASE = \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.c \

PLUG_LTE_TELIT_HFILES = \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.h \

PLUG_NR_5G_TELIT_OBJS = \
    plug_NR_5G_telit_test.o \
    plug_NR_5G_telit_lib.o \
	plug_NR_5G_telit_util.o \
	plug_NR_5G_telit_host.o \

PLUG_NR_5G_TELIT_BASE = \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_test.c \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_lib.c \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_util.c \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_host.c \

PLUG_NR_5G_TELIT_HFILES = \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_test.h \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_lib.h \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_util.h \
    $(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_host.h \


#
#
# Pluggable Serial(host)
# 
PLUG_SERIAL_OBJS = \
    platform_plug_serial_test.o \
	platform_plug_serial_util.o \
	platform_plug_serial_host.o \
	plug_serial_host_impl.o

PLUG_SERIAL_BASE = \
    $(PLUG_SERIAL_SRC)/platform_plug_serial_test.c \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_util.c \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_host.c \
	$(PROJ_DIR)/plug_serial_host_impl.c

PLUG_SERIAL_HFILES = \
    $(PLUG_SERIAL_SRC)/platform_plug_serial_test.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_util.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_host.h \
	$(PROJ_DIR)/plug_serial_host_impl.h

PLUG_OBJS = \
    diag_sirius_fpga_lib.o \
	diag_sirius_fpga_util.o \
	plug_host_fpga_lib.o \
	$(PLUG_COMMON_OBJS) \
	$(PLUG_TC_OBJS) \
	$(PLUG_LTE_OBJS) \
	$(PLUG_LTE_TELIT_OBJS) \
	$(PLUG_SERIAL_OBJS) \
	$(PLUG_NR_5G_TELIT_OBJS) \

PLUG_BASE = \
    $(PROJ_DIR)/diag_sirius_fpga_lib.c \
	$(PROJ_DIR)/diag_sirius_fpga_util.c \
	$(PROJ_DIR)/plug_host_fpga_lib.c \
	$(PLUG_COMMON_BASE) \
	$(PLUG_TC_BASE) \
	$(PLUG_LTE_BASE) \
	$(PLUG_LTE_TELIT_BASE) \
	$(PLUG_SERIAL_BASE) \
	$(PLUG_NR_5G_TELIT_BASE) \

PLUG_HFILES = \
    $(PROJ_DIR)/diag_sirius_fpga_lib.h \
	$(PROJ_DIR)/diag_sirius_fpga_util.h \
	$(PROJ_DIR)/plug_host_fpga_lib.h \
	$(PLUG_COMMON_HFILES) \
	$(PLUG_TC_HFILES) \
	$(PLUG_LTE_HFILES) \
	$(PLUG_LTE_TELIT_HFILES) \
	$(PLUG_SERIAL_HFILES) \
	$(PLUG_NR_5G_TELIT_HFILES) \


CPSS_OBJS = \
	cpss_common.o \
	cpss_extserv.o \
	cpss_os.o \

CPSS_BASE = \
	$(CPSS_SRC)/cpss_common.c \
	$(PROJ_DIR)/cpss_extserv.c \
	$(CPSS_SRC)/cpss_os.c \



#
# Common USB Dongle
# 
USB_DONG_COMMON_OBJS = \
	usb_dongle_host_slot_modules.o \
    usb_dongle_common_test.o \
	usb_dongle_common_lib.o \
	usb_dongle_common_host.o \
	usb_dongle_common_host_impl.o \

USB_DONG_COMMON_BASE = \
    $(USB_DONG_COMMON_SRC)/usb_dongle_common_test.c \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_lib.c \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_host.c \
	$(PROJ_DIR)/usb_dongle_common_host_impl.c \
	$(PROJ_DIR)/usb_dongle_host_slot_modules.c \

USB_DONG_COMMON_HFILES = \
    $(USB_DONG_COMMON_SRC)/usb_dongle_common_test.h \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_lib.h \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_host.h \
	$(PROJ_DIR)/usb_dongle_common_host_impl.h \
	$(PROJ_DIR)/usb_dongle_host_slot_modules.h \

##
## LTE(SWI) USB Dongle
## 
USB_DONG_LTE_SWI_OBJS = \
	usb_dongle_lte_swi_test.o \
	usb_dongle_lte_swi_lib.o \
	usb_dongle_lte_swi_util.o \

USB_DONG_LTE_SWI_BASE = \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_test.c \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_lib.c \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_util.c \

USB_DONG_LTE_SWI_HFILES = \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_test.h \
    $(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_lib.h \
    $(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_util.h \


USB_DONG_OBJS = \
	$(USB_DONG_COMMON_OBJS) \
	$(USB_DONG_LTE_SWI_OBJS) \

USB_DONG_BASE = \
	$(USB_DONG_COMMON_BASE) \
	$(USB_DONG_LTE_SWI_BASE) \

USB_DONG_HFILES = \
	$(USB_DONG_COMMON_HFILES) \
	$(USB_DONG_LTE_SWI_HFILES) \


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
    linux_coretest.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pci.o \
    linux_pcie.o \
    linux_pciutils.o \
    memops.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	pca.o \
	srec2bin.o
	
COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/menu.c \
    $(COMSRCDIR)/common_utils.c \
    $(COMSRCDIR)/print_hex.c \
    $(COMSRCDIR)/query.c \
    $(COMSRCDIR)/linux_pci.c \
    $(COMSRCDIR)/linux_pcie.c \
    $(COMSRCDIR)/linux_pciutils.c \
    $(COMSRCDIR)/memops.c \
    $(COMSRCDIR)/testmem.c \
    $(COMSRCDIR)/bit_swaps.c \
    $(COMSRCDIR)/tftpclient.c \
    $(COMSRCDIR)/cookie_4_core.c \
	$(COMSRCDIR)/tam_act2_utils.c \
	$(COMSRCDIR)/tam_aikido_upgrade.c \
	$(COMSRCDIR)/pca.c \
	$(COMSRCDIR)/srec2bin.c \

COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_coretest.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
    $(INCDIR)/common_utils.h \
    $(INCDIR)/linux_usb_test.h \
	$(INCDIR)/linux_pciutils.h \
	$(INCDIR)/linux_pci.h \
    $(INCDIR)/i2c_dev.h \
    $(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing.h \
	$(LIB_TAM_ACT2_SRC)/tam_library.h \
    $(LTC_SRC)/dev_ltc4215.h

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files :
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	diag_i2c_util.o \
	platform_i2c.o \
	diag_usb_lib.o \
	diag_usb_test.o \
	diag_spi_test.o \
	diag_rtc_lib.o \
	diag_rtc_util.o \
	diag_rtc_test.o \
	diag_temp_sensor_lib.o \
	diag_temp_sensor_util.o \
	diag_temp_sensor_test.o \
	linux_popen.o \
    diag_mother_board_test.o \
    diag_uart_lib.o \
    platform_stub.o \
    linux_ntwk.o \
    diag_poe_psu_lib.o \
    diag_poe_psu_util.o \
    diag_poe_psu_test.o \
    diag_moka_fpga_lib.o \
    diag_moka_fpga_test.o \
    diag_moka_fpga_util.o \
    diag_aikido_fpga_lib.o \
    diag_aikido_fpga_test.o \
    diag_aikido_fpga_util.o \
    diag_sirius_fpga_test.o \
    platform_cookie.o \
	diag_cpu_lib.o \
	diag_cpu_util.o \
	diag_cpu_test.o \
    tam_act2_api_drv_support.o \
    diag_smi_lib.o \
    diag_ge_phy_test.o \
    diag_ge_phy_lib.o \
    diag_ge_phy_util.o \
    diag_esw_test.o \
    diag_esw_lib.o \
    diag_esw_util.o \
	diag_dimm_lib.o \
	diag_dimm_util.o \
    diag_mcu_util.o \
    diag_mcu_lib.o \
	diag_wifi_lib.o \
	diag_wifi_util.o \
	diag_wifi_test.o \
	diag_led_util.o \
	diag_led_test.o \
    tam_aikido_upgrade.o \
	diag_eth_pkt_txrx.o\
	diag_eth_pkt_txrx_api.o\
	diag_eth_pkt_txrx_utils.o\
	$(PLUG_OBJS) \
	diag_emmc_lib.o \
	diag_emmc_util.o \
	diag_emmc_test.o \
    diag_reset_button_test.o \
    diag_reset_button_lib.o \
    $(USB_DONG_OBJS) \

PLAT_BASE = \
	$(PROJ_DIR)/linux_main.c \
	$(PROJ_DIR)/diag.c \
	$(PROJ_DIR)/diag_i2c_lib.c \
    $(PROJ_DIR)/platform_i2c.c \
    $(PROJ_DIR)/diag_usb_lib.c \
    $(PROJ_DIR)/diag_usb_test.c \
    $(PROJ_DIR)/diag_spi_test.c \
    $(PROJ_DIR)/diag_rtc_lib.c \
    $(PROJ_DIR)/diag_rtc_util.c \
    $(PROJ_DIR)/diag_rtc_test.c \
    $(PROJ_DIR)/diag_temp_sensor_lib.c \
    $(PROJ_DIR)/diag_temp_sensor_util.c \
    $(PROJ_DIR)/diag_temp_sensor_test.c \
    $(COMSRCDIR)/linux_popen.c \
	$(PROJ_DIR)/diag_mother_board_test.c \
	$(PROJ_DIR)/diag_uart_lib.c \
    $(PROJ_DIR)/platform_stub.c \
    $(COMSRCDIR)/linux_ntwk.c \
    $(PROJ_DIR)/diag_poe_psu_lib.c \
    $(PROJ_DIR)/diag_poe_psu_util.c \
    $(PROJ_DIR)/diag_poe_psu_test.c \
    $(PROJ_DIR)/diag_moka_fpga_lib.c \
    $(PROJ_DIR)/diag_moka_fpga_test.c \
    $(PROJ_DIR)/diag_moka_fpga_util.c \
    $(PROJ_DIR)/diag_aikido_fpga_lib.c \
    $(PROJ_DIR)/diag_aikido_fpga_test.c \
    $(PROJ_DIR)/diag_aikido_fpga_util.c \
    $(PROJ_DIR)/diag_sirius_fpga_test.c \
    $(PROJ_DIR)/tam_act2_api_drv_support.c \
	$(PROJ_DIR)/platform_cookie.c \
	$(PROJ_DIR)/diag_cpu_lib.c \
	$(PROJ_DIR)/diag_cpu_util.c \
	$(PROJ_DIR)/diag_cpu_test.c \
    $(PROJ_DIR)/diag_smi_lib.c \
    $(PROJ_DIR)/diag_ge_phy_test.c \
    $(PROJ_DIR)/diag_ge_phy_lib.c \
    $(PROJ_DIR)/diag_ge_phy_util.c \
    $(PROJ_DIR)/diag_esw_test.c \
    $(PROJ_DIR)/diag_esw_lib.c \
    $(PROJ_DIR)/diag_esw_util.c \
    $(PROJ_DIR)/diag_eth_pkt_txrx.c \
    $(PROJ_DIR)/diag_eth_pkt_txrx_api.c \
    $(PROJ_DIR)/diag_eth_pkt_txrx_utils.c \
	$(PROJ_DIR)/diag_dimm_lib.c \
	$(PROJ_DIR)/diag_dimm_util.c \
    $(PROJ_DIR)/diag_mcu_util.c \
    $(PROJ_DIR)/diag_mcu_lib.c \
	$(PROJ_DIR)/diag_wifi_lib.c \
	$(PROJ_DIR)/diag_wifi_util.c \
	$(PROJ_DIR)/diag_wifi_test.c \
	$(PROJ_DIR)/diag_led_util.c \
	$(PROJ_DIR)/diag_led_test.c \
	$(PLUG_BASE) \
	$(PROJ_DIR)/diag_emmc_lib.c \
	$(PROJ_DIR)/diag_emmc_util.c \
	$(PROJ_DIR)/diag_emmc_test.c \
    $(PROJ_DIR)/diag_reset_button_test.c \
    $(PROJ_DIR)/diag_reset_button_lib.c \
    $(USB_DONG_BASE) \

PLAT_HFILES = \
    $(PROJ_DIR)/mon_plat_defs.h \
    $(PROJ_DIR)/cookie_plat.h \
    $(PROJ_DIR)/linux_main.h \
    $(PROJ_DIR)/diag_mother_board_test.h \
    $(PROJ_DIR)/ngio.h \
    $(PROJ_DIR)/platform_i2c.h \
    $(PROJ_DIR)/diag_usb_lib.h \
    $(PROJ_DIR)/diag_usb_test.h \
    $(PROJ_DIR)/diag_spi_test.h \
    $(PROJ_DIR)/diag_i2c_lib.h \
    $(PROJ_DIR)/diag_i2c_test.h \
    $(PROJ_DIR)/diag_i2c_util.h \
	$(PROJ_DIR)/pcmap.h \
	$(PROJ_DIR)/nvmonvars.h \
	$(PROJ_DIR)/object.h \
	$(PROJ_DIR)/diag_rtc_lib.h \
	$(PROJ_DIR)/diag_rtc_util.h \
	$(PROJ_DIR)/diag_rtc_test.h \
	$(PROJ_DIR)/diag_temp_sensor_lib.h \
	$(PROJ_DIR)/diag_temp_sensor_util.h \
	$(PROJ_DIR)/diag_temp_sensor_test.h \
	$(PROJ_DIR)/diag_uart_lib.h \
	$(PROJ_DIR)/platform_stub.h \
    $(INCDIR)/linux_ntwk.h \
    $(PROJ_DIR)/diag_poe_psu_lib.h \
    $(PROJ_DIR)/diag_poe_psu_util.h \
    $(PROJ_DIR)/diag_poe_psu_test.h \
    $(PROJ_DIR)/act2_utils.h \
	$(PROJ_DIR)/act2l_typedef.h \
	$(PROJ_DIR)/platform_cookie.h \
	$(PROJ_DIR)/tam_act2_api_drv_support.h \
    $(PROJ_DIR)/diag_smi_lib.h \
    $(PROJ_DIR)/diag_esw_test.h \
    $(PROJ_DIR)/diag_esw_lib.h \
    $(PROJ_DIR)/diag_esw_util.h \
    $(PROJ_DIR)/diag_ge_phy_test.h \
    $(PROJ_DIR)/diag_ge_phy_lib.h \
    $(PROJ_DIR)/diag_ge_phy_util.h \
    $(PROJ_DIR)/diag_moka_fpga_lib.h \
    $(PROJ_DIR)/diag_moka_fpga_test.h \
    $(PROJ_DIR)/diag_moka_fpga_util.h \
    $(PROJ_DIR)/diag_aikido_fpga_lib.h \
    $(PROJ_DIR)/diag_aikido_fpga_test.h \
    $(PROJ_DIR)/diag_aikido_fpga_util.h \
    $(PROJ_DIR)/diag_sirius_fpga_test.h \
    $(PROJ_DIR)/diag_eth_pkt_txrx.h \
    $(PROJ_DIR)/diag_eth_pkt_txrx_api.h \
    $(PROJ_DIR)/diag_eth_pkt_txrx_utils.h \
    $(PROJ_DIR)/diag_eth_info.h \
	$(PROJ_DIR)/diag_dimm_lib.h \
	$(PROJ_DIR)/diag_dimm_util.h \
	$(PROJ_DIR)/diag_mcu_util.h \
	$(PROJ_DIR)/diag_mcu_lib.h \
	$(PROJ_DIR)/diag_wifi_lib.h \
	$(PROJ_DIR)/diag_wifi_util.h \
	$(PROJ_DIR)/diag_wifi_test.h \
	$(PROJ_DIR)/diag_cpu_lib.h \
	$(PROJ_DIR)/diag_cpu_util.h \
	$(PROJ_DIR)/diag_cpu_test.h \
	$(PROJ_DIR)/diag_enhance_err_msg_lib.h \
	$(PLUG_HFILES) \
	$(PROJ_DIR)/diag_led_util.h \
	$(PROJ_DIR)/diag_led_test.h \
	$(PROJ_DIR)/diag_emmc_lib.h \
	$(PROJ_DIR)/diag_emmc_util.h \
	$(PROJ_DIR)/diag_emmc_test.h \
    $(PROJ_DIR)/diag_reset_button_test.h \
    $(PROJ_DIR)/diag_reset_button_lib.h \
    $(USB_DONG_HFILES) \

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#

CPSS_OTHER_OBJS = \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/cpssHwInit/cpssHwInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/cpssHwInit/prvCpssHwMultiPortGroups.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/cpssHwInit/prvCpssHwRegisters.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/cpssHwInit/prvCpssCommonCpssHwInitLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/config/cpssGenCfg.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/config/prvCpssCommonConfigLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/systemRecovery/cpssGenSystemRecovery.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/systemRecovery/prvCpssCommonSystemRecoveryLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/init/cpssInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/init/prvCpssCommonInitLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/private/prvCpssGlobalMutex.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/private/prvCpssCyclicLogger.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/private/utils/prvCpssGenUtils.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/diag/prvCpssCommonDiag.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/port/prvCpssCommonPortIfModeCfg.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/port/cpssCommonPortManager.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/port/cpssCommonPortSerdes.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/port/cpssCommonPortPcsCfg.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/common/coCpu/mvHwsPortApCoCpuIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/common/configElementDb/mvCfgSeqExecuter.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/common/os/mvSemaphore.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/common/siliconIf/mvHwsSiliconIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsIpcApis.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortApCtrlDiagIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortApCtrlIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortApIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortCfgIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortInitIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mvHwsPortIpcIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/apEngine/mvZ80Code.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/common/mvHwsDiagnostic.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/common/mvHwsPortModeElements.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mvHwsMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/cgMac/mvHwsCgMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/cgMac/mvHwsCgMac28nmDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/cgMac/mvHwsCgMac28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/geMac/mvHwsGeMac28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/geMac/mvHwsGeMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/geMac/mvHwsGeMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/geMac/mvHwsGeMacSgIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/hglMac/mvHwsHglMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/hglMac/mvHwsHglMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/hglMac/mvHwsHglMacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/ilknMac/mvHwsIlknMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/ilknMac/mvHwsIlknMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/tcamMac/mvHwsTcamMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/tcamMac/mvHwsTcamMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xgMac/mvHwsXgMac28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xgMac/mvHwsXgMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xgMac/mvHwsXgMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xgMac/mvHwsXgMacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMac28nmDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMac28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMacDbRev3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/xlgMac/mvHwsXlgMacRev3If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/d2dMac/mvHwsD2dMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti100Mac/mvHwsMti100MacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti100Mac/mvHwsMti100MacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti100Mac/mvHwsMti100MacBrRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti100Mac/mvHwsMti100MacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti400Mac/mvHwsMti400MacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti400Mac/mvHwsMti400MacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mti400Mac/mvHwsMti400MacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mtiCpuMac/mvHwsMtiCpuMacIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mtiCpuMac/mvHwsMtiCpuMacDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/mac/mtiCpuMac/mvHwsMtiCpuMacRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mvHwsPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/cgPcs/mvHwsCgPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/cgPcs/mvHwsCgPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/cgPcs/mvHwsCgPcsIf_Rev3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/cgPcs/mvHwsCgPcs28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/cgPcs/mvHwsCgPcs28nmDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/gPcs/mvHwsGPcs28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/gPcs/mvHwsGPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/gPcs/mvHwsGPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/gPcs/mvHwsGPcsIf_v2.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/hglPcs/mvHwsHglPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/hglPcs/mvHwsHglPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/ilknPcs/mvHwsIlknPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/ilknPcs/mvHwsIlknPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcs28nmDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcs28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcsDb_v3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcsIf_v2_0.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mmPcs/mvHwsMMPcsIf_v3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/tcamPcs/mvHwsTcamPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/tcamPcs/mvHwsTcamPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/xPcs/mvHwsXPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/xPcs/mvHwsXPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/d2dPcs/mvHwsD2dPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs100Db.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs50Db.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs50If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs100If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs200Db.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs200If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs400Db.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs400If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiCpuPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiCpuPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs50Rev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs100Rev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs400Rev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiUsxLowSpeedPcsRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiCpuPcsRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiPcs25Rev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiUsxPcsRev2If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/pcs/mtiPcs/mvHwsMtiCpuSgPcsIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/mvHwsSerdesIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/mvAvagoSerdesIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/aacs.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/aacs_server.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/aapl_core.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/avsp.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/diag_core.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/dfe.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/eye.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/eye_math.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/gpio_mdio.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/i2c.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/library.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/logging.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/mdio.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/meas.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/pmd.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/pmro.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/sbm.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/sbus.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/sbus_access.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/sensor.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/serdes_core.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/spico.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/spico_diag.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/user_code.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/mv_hws_avago_serdes_if.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/mv_hws_memmove.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/mv_hws_strtol.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdInitialization.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdUtils.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mvComphySerdesIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mvComphySerdesDbs.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C12GP4/mcesdC12GP4_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C12GP4/mcesdC12GP4_DeviceInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C12GP4/mcesdC12GP4_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C12GP4/mcesdC12GP4_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C28GP4X2/mcesdC28GP4X2_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C28GP4X2/mcesdC28GP4X2_DeviceInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C28GP4X2/mcesdC28GP4X2_FwDownload.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C28GP4X2/mcesdC28GP4X2_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C28GP4X2/mcesdC28GP4X2_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GP5X4/mcesdC56GP5X4_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GP5X4/mcesdC56GP5X4_FwDownload.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GP5X4/mcesdC56GP5X4_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GP5X4/mcesdC56GP5X4_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GP5X4/mcesdC56GP5X5_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GX4/mcesdC56GX4_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GX4/mcesdC56GX4_DeviceInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GX4/mcesdC56GX4_FwDownload.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GX4/mcesdC56GX4_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C56GX4/mcesdC56GX4_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GADCX4/mcesdC112GADCX4_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GADCX4/mcesdC112GADCX4_FwDownload.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GADCX4/mcesdC112GADCX4_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GADCX4/mcesdC112GADCX4_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GX4/mcesdC112GX4_API.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GX4/mcesdC112GX4_DeviceInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GX4/mcesdC112GX4_FwDownload.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GX4/mcesdC112GX4_HwCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhy/C112GX4/mcesdC112GX4_RegRW.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH/mvComPhyHDb_v1_0.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH/mvComPhyHEom.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH/mvComPhyHIf_v1_0.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH28nm/mvComPhyH28nmDb.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH28nm/mvComPhyH28nmEom.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH28nm/mvComPhyH28nmIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3Db.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyHRev2/mvComPhyHDb_v2_0.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/comPhyHRev2/mvComPhyHIf_v2_0.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/serdes/gw16/mvGw16If.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/silicon/general/mvHwsGeneralPortInitIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/silicon/general/mvHwsGeneralPortCfgIf.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/silicon/general/mvHwsGeneralCpll.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/silicon/serviceCpu/mvHwsServiceCpuFw.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/labServices/port/gop/src/port/silicon/serviceCpu/mvHwsServiceCpuInt.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvSrvCpuFirmware.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuIpcChannel.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuDrv_Msys.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuDrv_BobK.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/prvCpssGenericSrvCpuDrv_CM3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/srvCpu/private/prvCpssGenSrvCpuIpc.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/smi/cpssGenSmi.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/smi/prvSmiHwDriver.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/smi/prvXSmiHwDriver.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/smi/prvCpssSmiLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/i2c/cpssCommonPpI2cHwDriver.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/common/i2c/prvCpssI2cLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/driver/interrupts/cpssDrvComIntSvcRtn.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/extServices/cpssExtServices.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/events/cpssGenEventRequests.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/ipc/mvShmIpc.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/hwDriver/cpssHwDriverAPI.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/hwDriver/cpssHwDriverGenMmap.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/hwDriver/cpssHwDriverGenMmapAddrCompl8.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/hwDriver/cpssHwDriverGenMmapAddrCompl4.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/hwDriver/cpssHwDriverSip5SlaveSMI.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/log/cpssLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/log/prvCpssGenCommonTypesLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpss/generic/log/prvCpssGenCommonTypesManualLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/cpssBuffManagerPool.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/cpssFormatConvert.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssAvlTree.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssDevMemManager.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssMemLib.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssMisc.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssMth.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/prvCpssSkipList.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlAttr.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlEntity.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlFile.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlIndex.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlNode.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlPrivate.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlSearch.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlSet.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssCommon/mxml/prvCpssMxmlString.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/config/cpssDrvPpConGenInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/config/cpssDrvPpGenDump.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/config/cpssDrvPpConInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/cpssDriverPpHw.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/prvCpssDrvPpHwApi.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/cpssDrvPpHwPciCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/cpssDrvPpHwPciCntlStandBy.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/cpssDrvPpHwPexMbusCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/newdrv_pexCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/newdrv_eagleCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/pci/newdrv_falconZCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/smi/cpssDrvPpHwSmiCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/hardware/twsi/cpssDrvPpHwTwsiCntl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntCtrl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntInterrupts.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDxChEventsUnify.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChLion2.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChBobcat2.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChXcat3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChBobcat3.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChBobK.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChAldrin.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChFalcon.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChFalconZ2.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChHawk.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChPhoenix.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChHarrier.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChAldrin2.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/dxExMx/cpssDrvPpIntDefDxChAlleyCat5.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/generic/cpssDrvPpIntGenCtrl.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/generic/cpssDrvPpIntGenInit.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/generic/cpssDrvPpIntGenInterrupts.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/pp/interrupts/generic/prvCpssDrvPpIntGenMiiEmulateInterrupts.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/log/prvCpssDriverLog.o \
	$(CPSS_TOOL_LIB_SWITCH_OTHER_SRC)/cpssDriver/log/prvCpssDriverManualLog.o \



NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_TAM_OBJ) \
	$(CPSS_OBJS) \
	$(CPSS_OTHER_OBJS) \
	$(CPSS_COMMON) \
    $(LIB_PCI_UTILS) \
    $(LIB_Z) \
    $(LIB_Z_128) \
	$(CPSS_PPDRV) \
	$(CPSS_MAD) \

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE) \
	$(CPSS_BASE) \

#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(POE_TPS2386X_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(PROJ_DIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(DEV_MAX31730): \
	$(LIB_TAM_ACT2_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(PLUG_SERIAL_SRC): \
	$(TELIT_LTE_SRC): \
    $(CPSS_SRC): \
	$(DREAMLINER_PLD_PATH): \
	$(CPSS_TOOL_LIB_SWITCH_SRC): \
	$(MRV_98DXC25X_SRC): \
	$(MRV_88E1680_SRC): \
	$(PLUG_NR_5G_TELIT_SRC): \
	$(NR_5G_TELIT_SRC): \
	$(USB_DONG_COMMON_SRC): \
	$(USB_DONG_LTE_SWI_SRC): \
	$(LTE_SWI_SRC): \

vpath %.h \
	$(GE_PHY_SRC): \
	$(POE_TPS2386X_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(PROJ_DIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(DEV_MAX31730): \
	$(LIB_TAM_ACT2_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(PLUG_SERIAL_SRC):\
	$(TELIT_LTE_SRC): \
    $(CPSS_SRC): \
	$(CPSS_TOOL_LIB_PHY_HEADER): \
	$(CPSS_TOOL_LIB_SWITCH_SRC): \
	$(MRV_98DXC25X_SRC): \
	$(MRV_88E1680_SRC): \
	$(PLUG_NR_5G_TELIT_SRC): \
	$(NR_5G_TELIT_SRC): \
	$(USB_DONG_COMMON_SRC): \
	$(USB_DONG_LTE_SWI_SRC): \
	$(LTE_SWI_SRC): \

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
	@echo "CROSS_COMPILE: " $(CROSS_COMPILE)
	@echo "IROOT: " $(IROOT)
	@echo "PROJ_DIR: " $(PROJ_DIR)
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
	sh $(IROOT)/utils/banner.sh ELIXIR
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




# DO NOT DELETE THIS LINE -- make depend depends on it.
# *-------------------------------------------------
#$Log: Makefile,v $
#Revision 1.2  2021/09/24 01:21:05  harrchan
#Collapse Elixir-branch to Main Trunk.
#
#Revision 1.1.2.15  2021/05/31 10:53:32  illiu
#Clean up code
#
#Revision 1.1.2.14  2021/04/22 02:24:23  harrchan
#Add new source path for tam library 3.4.24
#
#Revision 1.1.2.13  2021/04/19 07:41:11  harrchan
#Modify AIKIDO_WDC_OBJECT_NAME flag to WDC_OBJECT_NAME
#
#Revision 1.1.2.12  2021/04/12 08:42:20  illiu
#Modify path and file for cpss related stuff
#
#Revision 1.1.2.11  2021/03/22 07:57:26  harrchan
#Used AIKIDO_WDC_OBJECT_NAME flag to support WDC_Object naming.
#
#Revision 1.1.2.10  2021/02/02 01:40:45  harrchan
#Enable AIKIDO_CT_DEV_KEY flag on elixir platform
#
#Revision 1.1.2.9  2021/01/26 03:26:51  illiu
#Modify Makefile because of rename dreamliner_cpss42 folder and nim_dm prefix file
#
#Revision 1.1.2.8  2020/12/08 01:43:48  harrchan
#Support Sears PIM module on elixir
#
#Revision 1.1.2.7  2020/12/04 08:44:13  illiu
#Change the path of Marvell switch/phy library
#
#Revision 1.1.2.6  2020/11/27 03:18:52  illiu
#Add usb dongle feature into test item(External USB test)
#
#Revision 1.1.2.5  2020/11/10 06:43:20  harrchan
#Support PCIe test card on elixir
#
#Revision 1.1.2.4  2020/11/05 06:34:55  harrchan
#1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
#2.Remove some debug message on AC5 init process
#
#Revision 1.1.2.3  2020/10/06 02:07:56  illiu
#Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
#
#Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
#Remove DSL and GSHDSL relevant part
#
#Revision 1.1.2.1  2020/09/09 09:10:32  illiu
#First version which has been ported with Dreamliner and Marvell CPSS
#
#Revision 1.6  2019/06/24 07:21:37  wilbhuan
#Supported Pluggable Serial Module.
#
#Revision 1.5  2019/06/12 01:08:00  wilbhuan
#Re-opened the CDETS:CSCvq01828. Solved the daily build issue.
#
#Revision 1.4  2019/06/10 01:23:58  wilbhuan
#1. Solved issue with CDETS: CSCvq01828.
#2. Summary of CSCvq01828: For daily build purpose, there's an error will occur if execute the command "make depend". In order to fix this issue, we should delete some of nonexistent files in the Makefile.
#
#Revision 1.3  2019/05/14 09:44:09  sherliu2
#Support Hyerloop
#
#Revision 1.2.6.4  2019/03/11 13:10:46  shjung
#Modified image name for Hyperloop
#
#Revision 1.2.6.3  2019/02/25 06:19:25  sherliu2
#Support Hyperloop on Betelgeuse
#
#Revision 1.2.6.2  2019/02/23 03:45:43  sherliu2
#Sync up with main trunk
#
#Revision 1.2  2019/01/10 06:36:21  wilbhuan
#The beginning of Betelgeuse application code.
#
# *-------------------------------------------------
