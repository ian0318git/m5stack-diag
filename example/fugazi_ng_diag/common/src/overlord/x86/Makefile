# $Id: Makefile,v 1.104 2020/05/22 02:28:43 qingcwan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Informers NET Diagnostic Monitor.
#
# May 2008-2009, Shih-Nan Huang adapted from PPC Xformers.
#
# Copyright (c) 2009-2019 by Cisco Systems, Inc.
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
SERVER:=$(HOSTNAME)

ifeq ($(origin USER),undefined)
USER=Bozo
endif

default: all

TOOLS_VER	= c4.7.0-p1
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
IROOT		= ../../../..
INCDIR		= $(IROOT)/common/include
FPGA_PATH	= /auto/sp-engops/diags/pld/fpga_image

#
# The platform specific source directory
#

SRCDIR		= .
OVERLORDDIR     = ..
KLMDIR		= $(IROOT)/klm
#
# The processor specific source directory
#

PROCSRCDIR	= $(IROOT)/x86/src
PROCINCDIR	= $(IROOT)/x86/include

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include

CLK1_SRC	= $(DEV_DRV)/dev_idt_ics844s2416i
DIMM_SRC	= $(DEV_DRV)/dev_dimm
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel
ENV_SRC		= $(DEV_DRV)/dev_csco_10698
I2CMX_SRC	= $(DEV_DRV)/dev_pca9545a_ti
KERNDIR		= $(COMSRCDIR)/kernel
M29312DIR	= $(COMSRCDIR)/m29312
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
SNSR_SRC	= $(DEV_DRV)/dev_max1617a
Socrates 	= $(COMSRCDIR)/Socrates
TDM_PLL_SRC	= $(DEV_DRV)/dev_idt82v32021
USB_SRC		= $(DEV_DRV)/dev_cy7c64215_cypress
UTIL_DIR	= $(IROOT)/utils
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client
POE30_SRC = $(DEV_DRV)/dev_ilp
CANIS_SRC_PATH  = $(COMSRCDIR)/canis
WOODLAWN_SRC_PATH  = $(COMSRCDIR)/woodlawn
GOLDSCHLAGER_SRC_PATH = $(COMSRCDIR)/goldschlager
TIMINGCARD_SRC_PATH  = $(COMSRCDIR)/timingcard
SHEDIR_SRC_PATH = $(COMSRCDIR)/shedir
DRACO_SRC_PATH = $(COMSRCDIR)/draco
OAKENSHIELD_SRC_PATH = $(COMSRCDIR)/oakenshield
AQUILA_SRC_PATH = $(COMSRCDIR)/aquila
KALAMATA_SRC_PATH = $(COMSRCDIR)/kalamata
RBCP_LIB_PATH = $(COMSRCDIR)/rbcp_lib
PLDOVLD = /nfs/sp-engops/diags/pld/overlord
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/victory
DREAMLINER_SRC_PATH  = $(COMSRCDIR)/dreamliner
DREAMLINER_PHY_PATH  = $(COMSRCDIR)/dreamliner/phy
DREAMLINER_CPSS_PATH  = $(COMSRCDIR)/dreamliner/cpss
DREAMLINER_PLD_PATH = /auto/sp-engops/diags/pld/dreamliner
NIGHTWATCH_SRC_PATH = $(COMSRCDIR)/nightwatch

#
# Path to fpga images
#
PLDDIR			= /auto/sp-engops/diags/pld
PLDLIB			= $(PLDDIR)/lib
PATRIOT_FW_PATH         = /auto/sp-engops/diags/pld/patriot


# BCM GE switch related defines
#
# BCM_SDK_ROOT = /ws/ptong-sjc/proj-utah/bcmsdk/sdk-xgs-robo-cisco-isr-gh-sdk-6.4.1
BCM_SDK_ROOT = /auto/overlord/ovld-vendor-src/bcmsdk/bcm-gesw-sdk
BCM_SDK_INCDIR = $(BCM_SDK_ROOT)/include
BCM_SYSTEMS_DIR = $(BCM_SDK_ROOT)/systems

#
# The BCM *.a lib files are copied from the build directory of the BCM tree
# to the /auto/sp-engops/diags/pld/overlord/bcm_lib automatically when
# user run the build-for-ovld script in the BCM tree.
# BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
#
# BCM_LIB_DIR = /auto/sp-engops/diags/pld/overlord/bcm_lib
# pfix-orig BCM_LIB_DIR = /auto/utah/bcm-lib
BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
BCM_BLD_VERSION_OBJ = $(BCM_LIB_DIR)/version.o
BCM_PLATFORM_DEFINES_OBJ = $(BCM_LIB_DIR)/platform_defines.o

BCM_LDFLAGS =  -lpthread -lm -lrt -Wl,--relax
BCM_DEFS = \
	-DSYS_BE_PIO=0 -DSYS_BE_PACKET=0 -DSYS_BE_OTHER=0 \
	-DBCM_ESW_SUPPORT \
	-DUNIX \
	-DBCM_WARM_BOOT_SUPPORT_SW_DUMP \


# Cavium remote lib
#
CVMX_SDK_ROOT = /auto/overlord/ovld-vendor-src/cvmx2.2
CVMX_REMOTE_LIB_PATH = $(CVMX_SDK_ROOT)/host/remote-lib
CVMX_REMOTE_LIB = $(CVMX_REMOTE_LIB_PATH)/libocteon-remote.a

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(OVERLORDDIR) \
	-I$(PROCINCDIR) \
	-I$(KERNDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(CLK1_SRC) \
	-I$(CLK2_SRC) \
	-I$(CLKBUF_SRC) \
	-I$(DIMM_SRC) \
	-I$(EEPROM_SRC) \
	-I$(ENV_SRC) \
	-I$(I2CMX_SRC) \
	-I$(SNSR_SRC) \
	-I$(TDM_PLL_SRC) \
	-I$(USB_SRC) \
	-I$(POE30_SRC) \
	-I$(TIMINGCARD_SRC_PATH) \
	-I$(SHEDIR_SRC_PATH) \
	-I$(DRACO_SRC_PATH) \
	-I$(OAKENSHIELD_SRC_PATH) \
	-I$(AQUILA_SRC_PATH) \
	-I$(KALAMATA_SRC_PATH) \
	-I$(RBCP_LIB_PATH) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(TAMLIB)

INC     += \
	-I$(DREAMLINER_SRC_PATH) \
	-I$(DREAMLINER_PHY_PATH) \
	-I$(DREAMLINER_CPSS_PATH)/include/ \
	-I$(DREAMLINER_CPSS_PATH)/include/cisco \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/driver/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDr \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDrv/drivers \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os/gtOs \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/trace \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/oam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager\
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/extMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/i2c/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/init \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/interrupts/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/mirror/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/configElementDb \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/siliconIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/cgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/geMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/hglMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/ilknMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/tcamMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xlgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/cgPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/gPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/hglPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/ilknPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/mmPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/tcamPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/xPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyH \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyHRev2 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/smi \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/hardware \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/dxExMx \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/generic \

INC	+= -I$(BCM_SDK_INCDIR) -I$(BCM_SYSTEMS_DIR) -I$(CVMX_REMOTE_LIB_PATH)

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
#

DEFS	= \
	-DOVERLORD \
	-DLINUX_APP \
	-DDIAGMON \
	-DDRV_EXTENDED_REG_TEST \
	-DVIR_IS_PHY \
	-DL_ENDIAN \
	-DINFORMERS_LNX_APP \
	-DTFTP_CLIENT 

DREAMLINER_DEFS = \
	-DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS

# If BANNER is on, add DISPLAY_ENGINEERING_BANNER to the DEFS.
# `make clean`
# `make BANNER=on` to display the engineering banner
# `make` to ignore the banner
ifeq ($(BANNER),on)
DEFS                += -DDISPLAY_ENGINEERING_BANNER
endif

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#
CPPOPTS     	= -Wall -Werror $(INC) ${DEFS} ${BCM_DEFS} $(DREAMLINER_DEFS)
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
ASFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

#test, try to compile library in big-endian by ICC
NEW_CLI_OBJS = cli_cmd.o
NEW_CLI_BASE = cli_cmd.c
NEW_CLI_HFILES = cli_cmd.h

#LIBDIR		= $(IROOT)/lib
#LIBSRCDIR	= $(LIBDIR)/src
#LIB_AUTH	= smart_cookie_auth_c2_95_3-p11a-linux.a

# overlord is 64 bit
LIB_AUTH_DIR = /auto/sp-engops/diags/pld/lib/intel/Linux

# achanta is 32 bit lib, overlord is 64 bit
PLDLIB_INTEL_LINUX_ACHANTA = /auto/sp-engops/diags/pld/lib/intel/Linux

LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone_ovld_x86_64.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_mfg_library_ovld_x86_64_diags.a
LIB_ACT2 = $(PLDOVLD)/act2_lib64.a

#LIB_CRYPTO = $(PLDLIB_INTEL_LINUX_ACHANTA)/cryptoc_rsa_toolkit.a
#LIB_CERTC = $(PLDLIB_INTEL_LINUX_ACHANTA)/certc_rsa_toolkit.a
LIB_QUACK = $(PLDLIB_INTEL_LINUX_ACHANTA)/ec_rsa_toolkit.a

LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

# BCM GE Switch LIB
BCM_LIBS = $(wildcard $(BCM_LIB_DIR)/lib*.a)

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= o2x86_lnx
NETELF		= $(NETTARGET).elf

#
# Use -Qoption,bepostld,-verbose to turn on postld verbose 
# to see whether postld do the necessary big-endian global pointers swap
# POSTLDVERBOSE	= -Qoption,bepostld,-verbose
#
POSTLDVERBOSE	=
#RAMLDFLAGS	= -Xlinker -e -Xlinker -Tinformersram.link \

RAMLDFLAGS 	=




#
# Lib sources
#


CRYPTO_OBJS = \
	$(LIB_AUTH_DIR)/crypto_ecdsa.a \
	$(LIB_AUTH_DIR)/crypto_utilities.a \
        $(LIB_AUTH_DIR)/smart_cookie_auth.a

#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_OBJS = \
	dev_32021.o \
	dev_idt_ics844s2416i.o \
	dev_ltc4215.o \
	dev_object.o \
	dev_pca9545.o \
	dev_at24c0n.o \
	dev_dimm.o \
	dev_1337.o \
	dev_csco_10698.o \
	dev_ilp.o \
	dev_cy7c64215.o


DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

DEV_BASE = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.c \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
	$(DEV_DRV)/dev_object.c \
	$(TDM_PLL_SRC)/dev_32021.c \
	$(I2CMX_SRC)/dev_pca9545.c \
	$(EEPROM_SRC)/dev_at24c0n.c \
	$(DIMM_SRC)/dev_dimm.c \
	$(ENV_SRC)/dev_csco_10698.c \
	$(POE30_SRC)/dev_ilp.c \
	$(USB_SRC)/dev_cy7c64215.c \


DEV_HFILES = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.h \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
	$(DEV_DRV)/dev_object.h \
	$(TDM_PLL_SRC)/dev_32021.h \
	$(I2CMX_SRC)/dev_pca9545.h \
	$(EEPROM_SRC)/dev_at24c0n.h \
	$(DIMM_SRC)/dev_dimm.h \
	$(RTC_SRC)/dev_1337.h \
	$(ENV_SRC)/dev_csco_10698.h \
	$(POE30_SRC)/dev_ilp.h \
	$(USB_SRC)/dev_cy7c64215.h

#
# Common object files
#

COM_OBJS = \
	linux_main.o \
	linux_ntwk.o \
	linux_i2c.o \
	linux_pci.o \
	linux_pciutils.o \
	linux_error.o \
	linux_stub.o \
	linux_mem.o \
	linux_uart.o \
	linux_pstat.o \
	linux_api.o \
	linux_memory_tester.o \
	linux_memory_tester_utils.o \
	menu.o \
	pca.o \
	common_utils.o \
	cli_cmd.o \
	ngio.o \
	slot.o \
	platform_slot.o \
	print_hex.o \
	shcmd.o \
	alias.o \
	monitor.o \
	parsetoken.o \
	builtin.o \
	rtc.o \
	linux_pcie.o \
	query.o \
	memops.o \
	testmem.o \
	smart_cookie.o \
	crypto_credential.o \
	cookie_4_core.o \
	bit_swaps.o \
	i2c_drv.o \
	platform_intr_test.o \
	uio_utils.o \
	oir_ltc4215_api.o \
	linux_usb_test.o \
	tftpclient.o \
	tam_act2_utils.o \
	adapter_fpga.o

COM_BASE = \
	$(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_pciutils.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_uart.c \
	$(COMSRCDIR)/linux_pstat.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/linux_memory_tester.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/hwic_stub.c \
	$(COMSRCDIR)/slot.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/cli_cmd.c \
	$(COMSRCDIR)/shcmd.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/monitor.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/print_hex.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/rtc.c \
	$(COMSRCDIR)/linux_pcie.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/smart_cookie.c \
	$(COMSRCDIR)/crypto_credential.c \
	$(COMSRCDIR)/cookie_4_core.c \
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/oir_ltc4215_api.c \
	$(COMSRCDIR)/uio_utils.c \
	$(COMSRCDIR)/pca.c \
	$(COMSRCDIR)/tftpclient.c \
	$(COMSRCDIR)/tam_act2_utils.c \
	$(COMSRCDIR)/adapter_fpga.c


COM_HFILES = \
	$(INCDIR)/aglobal.h \
	$(INCDIR)/atypes.h \
	$(INCDIR)/bexterr.h \
	$(INCDIR)/bitdef.h \
	$(INCDIR)/bsafe.h \
	$(INCDIR)/bsfmacro.h \
	$(INCDIR)/bsfplatf.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/common.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/console.h \
	$(INCDIR)/cookie_4.h \
	$(INCDIR)/cpu.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/crypto_credential.h \
	$(INCDIR)/crypto_ecdsa.h \
	$(INCDIR)/defs.h \
	$(INCDIR)/dev_print.h \
	$(INCDIR)/dnld.h \
	$(INCDIR)/duart16552.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/error.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/extern.h \
	$(INCDIR)/flash.h \
	$(INCDIR)/free.h \
	$(INCDIR)/hwic_slot.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/ich9_i2c_regs.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_eth.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/menu.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/new_proto.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/oir_ltc4215_api.h \
	$(INCDIR)/pca9545a.h \
	$(INCDIR)/pca.h \
	$(INCDIR)/pci.h \
	$(INCDIR)/pm_utils.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/queryflags.h \
	$(INCDIR)/reason.h \
	$(INCDIR)/resizeob.h \
	$(INCDIR)/router_if.h \
	$(INCDIR)/s2681.h \
	$(INCDIR)/sgmii_defs.h \
	$(INCDIR)/signals.h \
	$(INCDIR)/smart_cookie_auth.h \
	$(INCDIR)/sm_patriot.h \
	$(INCDIR)/spi_cookie.h \
	$(INCDIR)/strings.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/sh.h \
	$(INCDIR)/slot.h \
	$(INCDIR)/smart_cookie.h \
	$(INCDIR)/sm_slot.h \
	$(INCDIR)/stdlibrf.h \
	$(INCDIR)/t1e1_common.h \
	$(INCDIR)/term.h \
	$(INCDIR)/testmem.h \
	$(INCDIR)/types.h \
	$(INCDIR)/uio_utils.h \
	$(INCDIR)/adapter_fpga.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h

# COM_BASE += $(COMSRCDIR)/bcm56321.c
# COM_BASE += $(COMSRCDIR)/example_fw_dnld.c

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	platform_stub.o \
	diag.o \
	env_mcu_firmware.o \
	VMON_MCU_01_01.o \
	igb_eeprom.o \
	ich9_i2c.o \
	platform_i2c.o \
	platform_i2c_usb.o \
	i2c_api.o \
	platform_env.o \
	platform_mcu.o \
	platform_margin_utils.o \
	platform_pwr_seq.o \
	platform_vtg_mntr.o \
	platform_sys_clk.o \
	platform_psu.o \
	platform_poe_psu.o \
	platform_pcie_clk.o \
	platform_sensor.o \
	platform_led.o \
	platform_eeprom.o \
	platform_eeprom_access.o \
	platform_prom.o \
	platform_30w_poe.o \
	platform_pll.o \
	platform_pcie_sw.o \
	platform_plx_pcie_sw.o \
        platform_pem_utils.o \
        platform_pem_monitor_read.o \
        platform_pem_eeprom_modify.o \
        platform_pem_fan.o \
        platform_idprom_utils.o \
	tam_act2_api_drv_support.o \
	platform_dimm.o \
	mb_tests.o \
	platform_cookie.o \
	platform_barometer.o \
	moncmds.o \
	dash_fpga.o \
	dash_reg_test.o \
	dash_get_addr.o \
	dash_mem.o \
	eth_pkt_utils.o 

PLAT_OBJS += \
	bcm_gesw_init.o \
	bcm_gesw_tests.o \
	bcm_gesw_utils.o \
	bcm_gesw.o \
	bcm_gesw_api.o \
	platform_eth_pkt_txrx.o \
	cavecreek_sgmii.o

PLAT_BASE = \
	linux_main.c \
	diag.c \
	mb_tests.c \
	platform_i2c.c \
	platform_idprom_utils.c \
	platform_pem_eeprom_modify.c \
	platform_pem_fan.c \
	platform_pem_utils.c \
	platform_pem_monitor_read.c \
	platform_stub.c \
	$(OVERLORDDIR)/env_mcu_firmware.c \
	$(OVERLORDDIR)/linux_ntwk.c \
	$(OVERLORDDIR)/platform_margin_utils.c \
	$(OVERLORDDIR)/platform_pwr_seq.c \
	$(OVERLORDDIR)/platform_vtg_mntr.c \
	$(OVERLORDDIR)/platform_sys_clk.c \
	$(OVERLORDDIR)/platform_psu.c \
	$(OVERLORDDIR)/platform_poe_psu.c \
	$(OVERLORDDIR)/platform_i2c_usb.c \
	$(OVERLORDDIR)/platform_pcie_clk.c \
	$(OVERLORDDIR)/platform_sensor.c \
	$(OVERLORDDIR)/platform_led.c \
	$(OVERLORDDIR)/platform_eeprom.c \
	$(OVERLORDDIR)/platform_eeprom_access.c \
	$(OVERLORDDIR)/platform_prom.c \
	$(OVERLORDDIR)/platform_30w_poe.c \
	$(OVERLORDDIR)/platform_pll.c \
	$(OVERLORDDIR)/platform_pcie_sw.c \
	$(OVERLORDDIR)/platform_plx_pcie_sw.c \
	$(OVERLORDDIR)/platform_intr_test.c \
	$(OVERLORDDIR)/tam_act2_api_drv_support.c \
	$(OVERLORDDIR)/platform_cookie.c \
	$(OVERLORDDIR)/platform_barometer.c \
	$(OVERLORDDIR)/moncmds.c \
	$(OVERLORDDIR)/i2c_drv.c \
	$(OVERLORDDIR)/ngio.c \
	$(OVERLORDDIR)/dash_fpga.c \
	$(OVERLORDDIR)/dash_reg_test.c \
	$(OVERLORDDIR)/dash_get_addr.c \
	$(OVERLORDDIR)/dash_mem.c \
	$(OVERLORDDIR)/platform_dimm.c \
	$(COMSRCDIR)/linux_usb_test.c \
	$(OVERLORDDIR)/eth_pkt_utils.c \
	$(OVERLORDDIR)/bcm_gesw_init.c \
	$(OVERLORDDIR)/bcm_gesw_tests.c \
	$(OVERLORDDIR)/bcm_gesw_utils.c \
	$(OVERLORDDIR)/bcm_gesw.c \
	$(OVERLORDDIR)/bcm_gesw_api.c \
	$(OVERLORDDIR)/platform_eth_pkt_txrx.c \
        $(OVERLORDDIR)/platform_slot.c \
	platform_mcu.c \


PLAT_BASE += \
	$(OVERLORDDIR)/cavecreek_sgmii.c

PLAT_HFILES = \
	mb_tests.h \
	cookie_plat.h \
	platform_mcu.h \
    platform_pem_utils.h \
    platform_idprom_utils.h \
    platform_idprom.h \
    platform_pem_fan.h \
	platform_i2c.h \
	platform_fru.h \
	i2c_address.h \
	$(OVERLORDDIR)/act2_utils.h \
	$(OVERLORDDIR)/act2l_typedef.h \
	$(OVERLORDDIR)/ata_dev_dvr.h \
	$(OVERLORDDIR)/bcm_gesw.h \
	$(OVERLORDDIR)/bcm_gesw_defs.h \
	$(OVERLORDDIR)/ich_i2c.h \
	$(OVERLORDDIR)/linux_ntwk.h \
	$(OVERLORDDIR)/mfg_api.h \
	$(OVERLORDDIR)/mon_plat_defs.h \
	$(OVERLORDDIR)/nvmonvars.h \
	$(OVERLORDDIR)/octeon_test.h \
	$(OVERLORDDIR)/pcmap.h \
	$(OVERLORDDIR)/platform_30w_poe.h \
	$(OVERLORDDIR)/platform_pcie_sw.h \
	$(OVERLORDDIR)/platform_plx_pcie_sw.h \
	$(OVERLORDDIR)/platform_barometer.h \
	$(OVERLORDDIR)/platform_cookie.h \
	$(OVERLORDDIR)/platform_debug.h \
	$(OVERLORDDIR)/platform_eeprom_access.h \
	$(OVERLORDDIR)/platform_env.h \
	$(OVERLORDDIR)/platform_eth_pkt_txrx.h \
	$(OVERLORDDIR)/platform_i2c_usb.h \
	$(OVERLORDDIR)/platform_intr_test.h \
	$(OVERLORDDIR)/platform_led.h \
	$(OVERLORDDIR)/platform_pci.h \
	$(OVERLORDDIR)/platform_pcie_clk.h \
	$(OVERLORDDIR)/platform_poe_psu.h \
	$(OVERLORDDIR)/platform_prom.h \
	$(OVERLORDDIR)/platform_psu.h \
	$(OVERLORDDIR)/platform_pwr_seq.h \
	$(OVERLORDDIR)/platform_sensor.h \
	$(OVERLORDDIR)/platform_smi.h \
	$(OVERLORDDIR)/platform_sys_clk.h \
	$(OVERLORDDIR)/platform_usb_fw.h \
	$(OVERLORDDIR)/platform_vtg_mntr.h \
	$(OVERLORDDIR)/platform_margin_utils.h \
	$(OVERLORDDIR)/dash_fpga.h \
	$(OVERLORDDIR)/eth_pkt_utils.h \
	$(OVERLORDDIR)/ngio.h \
	$(OVERLORDDIR)/plat_defs.h \
	$(OVERLORDDIR)/platform_dimm.h \
        $(OVERLORDDIR)/platform_slot.h \
	$(OVERLORDDIR)/uart_fpga.h

PATRIOT_OBJS = \
	sm_patriot.o \
	sm_patriot_test.o \
	sm_patriot_utils.o \

PATRIOT_BASE = \
	$(COMSRCDIR)/sm_patriot.c \
	$(COMSRCDIR)/sm_patriot_test.c \
	$(COMSRCDIR)/sm_patriot_utils.c \

PATRIOT_HFILES = \
	$(INCDIR)/sm_patriot.h

LEBOWSKI_OBJS = \
	lebowski_host.o

LEBOWSKI_BASE = \
	$(COMSRCDIR)/lebowski_host.c

LEBOWSKI_HFILES = \
	$(INCDIR)/lebowski_host.h

NIGHTWATCH_OBJS = \
	nightwatch_host.o \
	nightwatch_diag_shell_api.o \
	pty.o

NIGHTWATCH_BASE = \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_host.c \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_diag_shell_api.c \
	$(NIGHTWATCH_SRC_PATH)/pty.c

NIGHTWATCH_HFILES = \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_host.h \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_diag_shell_api.h \
	$(NIGHTWATCH_SRC_PATH)/pty.h

FORTITUDE_OBJS = ngwic_fortitude.o
GRIMLOCK_OBJS = nim_grimlock.o

FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c 
GRIMLOCK_BASE = $(COMSRCDIR)/nim_grimlock.c

DREAMLINER_OBJS = \
	ngwic_dreamliner.o \
	dreamliner_phy.o \
	dreamliner_fpga.o \
	dreamliner_poe.o \
	dreamliner_ge_switch.o \
	dreamliner_xcat2_init.o \
	dreamliner_xcat2_utils.o \
	dreamliner_fpga_fw.o \
	nim_dm_cpss_extdrv.o \
	nim_dm_cpss_extserv.o \
	nim_dm_cpss_os.o \
	nim_dm_cpss_trace.o \


DREAMLINER_BASE = \
	$(DREAMLINER_SRC_PATH)/ngwic_dreamliner.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_phy.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_fpga.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_poe.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_ge_switch.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_init.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_utils.c \
	$(DREAMLINER_PLD_PATH)/dreamliner_fpga_fw.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extdrv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extserv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_os.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_trace.c \

DREAMLINER_LIB = \
	$(DREAMLINER_PLD_PATH)/libdreamlinersdk.a

PRINCE_OBJS = ngwic_prince.o

PRINCE_BASE = $(COMSRCDIR)/ngwic_prince.c 

REVA_OBJS = ngwic_reva.o

REVA_BASE = $(COMSRCDIR)/ngwic_reva.c

THULE_OBJS = ngsm_thule.o

THULE_BASE = $(COMSRCDIR)/ngsm_thule.c

OVERDRIVE_OBJS = ngwic_overdrive.o

OVERDRIVE_BASE = $(COMSRCDIR)/ngwic_overdrive.c 

CANIS_OBJS = \
	sm_canis.o \
	canis_rbcp_main.o \
	canis_rbcp_lib.o \
	canis_rbcp_platform.o

CANIS_BASE = \
	$(CANIS_SRC_PATH)/sm_canis.c \
	$(CANIS_SRC_PATH)/canis_rbcp_main.c \
	$(CANIS_SRC_PATH)/canis_rbcp_lib.c \
	$(CANIS_SRC_PATH)/canis_rbcp_platform.c

WOODLAWN_OBJS = \
	sm_woodlawn.o \
	sm_woodlawn_uart.o \
	sm_woodlawn_comm.o \
	sm_woodlawn_fpga_lib.o

WOODLAWN_BASE = \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_uart.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_comm.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_fpga_lib.c

GOLDSCHLAGER_OBJS = \
	ngwic_goldschlager.o \
	ngwic_goldschlager_comm.o \
	bcm63268_lib.o

GOLDSCHLAGER_BASE = \
	$(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager.c \
	$(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager_comm.c \
	$(GOLDSCHLAGER_SRC_PATH)/bcm63268_lib.c 

TIMINGCARD_OBJS = \
	jbicomp.o \
	jbijtag.o \
	jbimain.o \
	jbistub.o \
	vm_timingcard.o \
	vm_timingcard_cpld_diag.o \
	vm_timingcard_cpld_lib.o \
	vm_timingcard_cpld_fw.o \
	vm_timingcard_pca9557_diag.o \
	vm_timingcard_pca9557_lib.o \
	vm_timingcard_zl3036x_lib.o \
	vm_timingcard_zl3036x_diag.o

TIMINGCARD_BASE = \
	$(TIMINGCARD_SRC_PATH)/jbicomp.c \
	$(TIMINGCARD_SRC_PATH)/jbijtag.c \
	$(TIMINGCARD_SRC_PATH)/jbimain.c \
	$(TIMINGCARD_SRC_PATH)/jbistub.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_diag.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_fw.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_pca9557_diag.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_pca9557_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_zl3036x_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_zl3036x_diag.c 
SHEDIR_OBJS = \
	ngwic_shedir.o \
	shedir_rbcp_main.o \
	rbcp_platform.o \
	rbcp_lib.o 

SHEDIR_BASE = \
	$(SHEDIR_SRC_PATH)/ngwic_shedir.c \
	$(SHEDIR_SRC_PATH)/shedir_rbcp_main.c \
	$(SHEDIR_SRC_PATH)/rbcp_platform.c \
	$(RBCP_LIB_PATH)/rbcp_lib.c

DRACO_OBJS = \
	ngsm_draco.o \
	draco_rbcp_main.o

DRACO_BASE = \
	$(DRACO_SRC_PATH)/ngsm_draco.c \
	$(DRACO_SRC_PATH)/draco_rbcp_main.c


AQUILA_OBJS = \
	ngsm_aquila.o \
	aquila_rbcp_main.o

AQUILA_BASE = \
	$(AQUILA_SRC_PATH)/ngsm_aquila.c \
	$(AQUILA_SRC_PATH)/aquila_rbcp_main.c

KALAMATA_OBJS = \
	nim_kalamata.o \
	kalamata_rbcp_main.o

KALAMATA_BASE = \
	$(KALAMATA_SRC_PATH)/nim_kalamata.c \
	$(KALAMATA_SRC_PATH)/kalamata_rbcp_main.c

OAKENSHIELD_OBJS = \
	ngsm_oakenshield.o

OAKENSHIELD_BASE = \
	$(OAKENSHIELD_SRC_PATH)/ngsm_oakenshield.c

GRAFFHAM_OBJS = ngvm_graffham.o

GRAFFHAM_BASE = $(COMSRCDIR)/ngvm_graffham.c

F35_OBJS = nim_f35.o

F35_BASE = $(COMSRCDIR)/nim_f35.c 

WALLANDER_OBJS = nim_wallander.o

WALLANDER_BASE = $(COMSRCDIR)/nim_wallander.c

ARKENSTONE_OBJS = nim_arkenstone.o

ARKENSTONE_BASE = $(COMSRCDIR)/nim_arkenstone.c 

#
# Test Card related files
#
TESTCARD_OBJS = \
	ngio_testcard.o \
	testcard_fpga.o \
	testcard_eth.o \
	testcard_xaui.o \
	testcard_pcie.o \
        testcard_plx_pcie_sw.o \
        testcard_tlk_10232.o \
	testcard_uart.o

TESTCARD_BASE = \
	$(OVERLORDDIR)/ngio_testcard.c \
	$(OVERLORDDIR)/testcard_fpga.c \
	$(OVERLORDDIR)/testcard_eth.c \
	$(OVERLORDDIR)/testcard_xaui.c \
	$(OVERLORDDIR)/testcard_pcie.c \
	$(OVERLORDDIR)/testcard_plx_pcie_sw.c \
	$(OVERLORDDIR)/testcard_tlk_10232.c \
	$(OVERLORDDIR)/testcard_uart.c

TESTCARD_HFILES = \
	$(OVERLORDDIR)/ngio_testcard.h \
	$(OVERLORDDIR)/testcard_fpga.h \
	$(OVERLORDDIR)/testcard_eth.h \
	$(OVERLORDDIR)/testcard_plx_pcie_sw.h \
	$(OVERLORDDIR)/testcard_tlk_10232.h \
	$(OVERLORDDIR)/testcard_xaui.h \
	$(OVERLORDDIR)/testcard_pcie.h

#
# Dynamo NIM
#
DYNAMO_OBJS = nim_dynamo.o
DYNAMO_BASE = $(COMSRCDIR)/nim_dynamo.c

#
# ShrinkRay SM
#
SHRINKRAY_OBJS = \
	shrinkray_host.o \
	shrinkray_utils.o

SHRINKRAY_BASE = \
	$(COMSRCDIR)/shrinkray_host.c \
	$(COMSRCDIR)/shrinkray_utils.c

SHRINKRAY_HFILES = \
	$(INCDIR)/shrinkray_host.h

#
# Skye SM
#
SKYE_OBJS = \
	skye_host.o \
	skye_utils.o

SKYE_BASE = \
	$(COMSRCDIR)/skye_host.c \
	$(COMSRCDIR)/skye_utils.c

SKYE_HFILES = \
	$(INCDIR)/skye_host.h

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	octeon_test.o \
	host_mbox.o \
	$(PATRIOT_OBJS) \
	$(CANIS_OBJS) \
	$(WOODLAWN_OBJS) \
	$(GOLDSCHLAGER_OBJS) \
	$(SHEDIR_OBJS) \
	$(DRACO_OBJS) \
	$(OAKENSHIELD_OBJS) \
	$(AQUILA_OBJS) \
	$(KALAMATA_OBJS) \
	$(FORTITUDE_OBJS) \
	$(GRIMLOCK_OBJS) \
	$(DREAMLINER_OBJS) \
	$(DREAMLINER_LIB) \
	$(PRINCE_OBJS) \
	$(REVA_OBJS) \
	$(OVERDRIVE_OBJS) \
	$(THULE_OBJS) \
	$(CVMX_REMOTE_LIB) \
	$(CRYPTO_OBJS) \
	$(LIB_PCI_UTILS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_QUACK) \
	$(GRAFFHAM_OBJS) \
	$(TESTCARD_OBJS) \
	$(LEBOWSKI_OBJS) \
	$(NIGHTWATCH_OBJS) \
	$(DYNAMO_OBJS) \
	$(SHRINKRAY_OBJS) \
	$(F35_OBJS) \
	$(WALLANDER_OBJS) \
	$(TIMINGCARD_OBJS) \
	$(SKYE_OBJS) \
	$(ARKENSTONE_OBJS)

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(M29312_BASE) \
	$(PLAT_BASE) \
	$(PSE2_BASE) \
	$(PVDM_BASE) \
	$(PSE_BASE) \
	$(PATRIOT_BASE) \
	$(FORTITUDE_BASE) \
	$(GRIMLOCK_BASE) \
	$(DREAMLINER_BASE) \
	$(PRINCE_BASE) \
	$(REVA_BASE) \
	$(OVERDRIVE_BASE) \
	$(THULE_BASE) \
	$(CANIS_BASE) \
	$(WOODLAWN_BASE) \
	$(GOLDSCHLAGER_BASE) \
	$(SHEDIR_BASE)      \
	$(DRACO_BASE)      \
	$(AQUILA_BASE) \
	$(KALAMATA_BASE) \
	$(GRAFFHAM_BASE) \
	$(TESTCARD_BASE) \
	$(LEBOWSKI_BASE) \
	$(NIGHTWATCH_BASE) \
	$(DYNAMO_BASE) \
	$(SHRINKRAY_BASE) \
	$(F35_BASE) \
	$(WALLANDER_BASE) \
	$(TIMINGCARD_BASE) \
	$(SKYE_BASE) \
	$(ARKENSTONE_BASE)

#
#########   Header Files #########
#

HFILES  = \
        $(COM_HFILES) \
        $(DEV_HFILES) \
	$(PATRIOT_HFILES) \
	$(TESTCARD_HFILES) \
        $(PLAT_HFILES) \
	$(LEBOWSKI_HFILES) \
	$(NIGHTWATCH_HFILES) \
	$(SHRINKRAY_HFILES) \
	$(SKYE_HFILES)

#
#########   UCODE (MicroCodes Files #########
#

UCODES	= \
	hwic_dsu_v3_fmw

#
#########   ASM Files #########
#

ASM	= \
	$(PLAT_AFILES)

vpath %.c \
	$(SRCDIR): \
	$(OVERLORDDIR): \
	$(COMSRCDIR): \
	$(PROCSRCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	\
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(CX27470): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DIMM_SRC): \
	$(DOBRO_FW_PATH): \
	$(DSP_5510_FW_PATH): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(ENZO_FPGA_PATH): \
	$(ENZO_PLUS_FPGA_PATH) \
	$(ENZO_UCODE): \
	$(ENZOPLUS_UCODE) \
	$(FPGA_IMAGE_PATH): \
	$(GE_QUAD_PHY_SRC): \
	$(GUIDO_FPGA_PATH): \
	$(I2CMX_SRC): \
	$(INFORMERS_FPGA_PATH): \
	$(JEBBIN_FPGA_PATH): \
	$(M29312DIR): \
	$(NEZHA_DIR): \
	$(PADME_FPGA_PATH): \
	$(PATH_OF_BORG_FW): \
	$(PATH_OF_DSL_FW): \
	$(PATH_OF_BORG_FPGA): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(PVDM_BASE): \
	$(RTC_SRC): \
	$(LM75_SRC): \
	$(SIMPSONS_FPGA_PATH): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(Socrates): \
	$(SOPRANO_FPGA_PATH): \
	$(SPIDEY_FPGA_PATH): \
	$(SPIDEY_PLD): \
	$(SPIDEY_T3_UCODE): \
	$(SPIDEY_UCODE): \
	$(TDM_PLL_SRC): \
	$(USB_SRC): \
	$(MONTALVO_FW_PATH): \
	$(MARIO_FPGA_PATH): \
	$(PSE_FPGA_PATH): \
	$(PRECISION_IMAGE_PATH): \
	$(SQUIER_IMAGE_PATH): \
	$(TRIPLE_SEC_FPGA_PATH): \
	$(PATH_OF_BCM_FW): \
	$(TROI_FPGA_PATH): \
	$(CX98124): \
	$(TROI_BASE): \
	$(FPGA_PATH): \
	$(PATRIOT_FW_PATH): \
	$(CANIS_SRC_PATH): \
	$(WOODLAWN_SRC_PATH): \
	$(GOLDSCHLAGER_SRC_PATH): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PLD_PATH): \
	$(DREAMLINER_PHY_PATH): \
	$(DREAMLINER_CPSS_PATH)/src: \
	$(PLDOVLD): \
	$(GOOFY): \
	$(SHEDIR_SRC_PATH): \
	$(DRACO_SRC_PATH): \
	$(OAKENSHIELD_SRC_PATH): \
	$(AQUILA_SRC_PATH): \
	$(KALAMATA_SRC_PATH): \
	$(RBCP_LIB_PATH): \
	$(TIMINGCARD_SRC_PATH): \
	$(NIGHTWATCH_SRC_PATH):

vpath %.h \
	$(SRCDIR): \
	$(OVERLORDDIR): \
	$(PROCINCDIR): \
	$(INCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	$(CHIPINC): \
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DEV_DRV)/dev_3086_infineon: \
	$(DEV_DRV)/dev_82912_infineon: \
	$(DIMM_SRC): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(GE_QUAD_PHY_SRC): \
	$(I2CMX_SRC): \
	$(M29312DIR): \
	$(NEZHA_INCDIR): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(LM75_SRC): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PHY_PATH): \
	$(RBCP_LIB_PATH): \
	$(TAMLIB):

# Override RCS stuff

#%: %,v

#%: RCS/%,v

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

#
# do not delete the last line for object dump just in case
# we need to do this when debugging a problem. temp comment out
#
#install: $(SRECFILE) $(NETTARGET) install.sh
#
# temp comment out ROM version due to compiler update problem
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
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : $(TARGET_DEPEND) $(HFILES)
	sh $(IROOT)/utils/banner.sh OVLD
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) \
	-Wl,--start-group $(BCM_PLATFORM_DEFINES_OBJ) $(BCM_BLD_VERSION_OBJ) $(BCM_LIBS) \
	-Wl,--end-group ${BCM_LDFLAGS}  $(LSPCI_LDFLAGS)

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols

#	$(CC) $(CFLAGS)
#	$(LD) -o $(TARGET_OBJS)
#        @sh $(IROOT)/utils/banner.sh INFORMERS
#        $(CC) $(CFLAGS) -c banner.c
#        $(LD) -o $(TARGET_OBJS)

#$(NETTARGET): $(TARGET_DEPEND)
#	$(CC) $(CFLAGS) $(TARGET_OBJS) -o $(NETTARGET)


#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES) $(ASM)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) $(ASM) > dependancies
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

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.104  2020/05/22 02:28:43  qingcwan
# Merge switzer-carrier code into main chunk.
#
# Revision 1.103  2020/03/13 12:14:21  letsai
# Support Grimlock NIM
#
# Revision 1.102  2020/01/09 01:02:28  jiajliu
# Merge Curie 2RU to main trunk
#
# Revision 1.101  2019/08/06 06:56:16  alpeng
# merge curie, switzer and nightwatch to trunk
#
# Revision 1.100  2018/08/06 02:37:59  harrchan
# Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)
#
# Revision 1.99  2018/05/22 02:31:12  alpeng
# fixed compiler warning, CSCvj57934
#
# Revision 1.98  2018/02/24 07:28:02  letsai
# Collapse Kalamata-branch to Main Trunk.
#
# Revision 1.95.4.1  2017/06/16 07:17:04  kodko
# Initial platform code commit for Kalamata project.
#
# Revision 1.97  2017/07/28 07:49:43  harrchan
# Collapse Oakenshield-branch to Main Trunk.
#
# Revision 1.96  2017/07/06 00:06:14  ptong
# Move TOOLS_VER from c4.5.3-p1 to c4.7.0-p1. Need to add -Wno-error=unused-but-set-variable flag to aviod massive changes in the diag tree.
#
# Revision 1.95  2017/03/21 08:41:57  olin2
# Collapse Aquila-branch to Main Trunk.
#
# Revision 1.94  2016/04/26 02:15:43  umlin
# Initial check-in for Reva.
# Merge Reva to maintrunk.
#
# Revision 1.93  2016/04/15 10:19:25  xiaoyizh
# Initial check-in for Arkenstone.
#
# Revision 1.92  2016/01/21 01:50:04  olin2
# Collapse Draco-branch to Main Trunk.
#
# Revision 1.91  2015/05/25 04:38:06  hondwang
# Fix make depend error for shedir nim
#
# Revision 1.90  2015/05/25 03:56:29  steja
# Add support Skye SM
#
# Revision 1.89  2015/05/18 03:35:12  hondwang
# fix make depend(shedir nim) error
#
# Revision 1.88  2015/05/14 03:36:11  hondwang
# Merge Shedir NIM to maintrunk
#
# Revision 1.87  2015/05/13 02:34:59  iachang
# Supported common TAM library include.
#
# Revision 1.86  2015/02/27 10:02:30  iachang
#
# Add support dreamliner NIM
#
# Revision 1.85  2015/02/14 12:48:41  kodko
# Collapse timing card branch code into main trunk.
#
# Revision 1.84  2015/02/13 02:33:47  iachang
# Supported Act2 TAM library
#
# Revision 1.83  2015/02/12 05:58:13  bowang3
# Add support to NIM Wallander
#
# Revision 1.82  2014/11/01 05:16:49  srane
# Add F35 NIM
#
# Revision 1.81  2014/10/02 18:31:07  ptong
# Use the latest BCM lib in /auto/overlord/ovld-vendor-src/bcmsdk/bcm-gesw-sdk
#
# Revision 1.80  2014/09/17 03:32:15  jamlin
# Add support for Goldschlager NIM.
#
# Revision 1.79  2014/08/18 22:15:14  yuetwang
# add engineering banner
#
# Revision 1.78  2014/07/01 09:15:54  bowang3
# Add support to NGSM carrier card Thule
#
# Revision 1.77  2014/06/19 22:00:52  ptong
# Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression
#
# Revision 1.76  2014/06/09 00:18:24  alpeng
# add new files for testcard
#
# Revision 1.75  2014/05/14 21:20:12  ptong
# o2x86_lnx now runs on kernel 3.8.3. GESW lib BCM sdk-xgs-robo-6.4.0
#
# Revision 1.74  2014/05/05 09:18:41  alpeng
# recover bcm lib to order version and wait for official release
#
# Revision 1.73  2014/05/05 09:13:05  alpeng
# upgrade bcm lib for tag ovld-juno-v10_0-050514 temporarily
#
# Revision 1.72  2014/03/26 19:23:22  siyen
# Added Dynamo supports at the platform (CSCun82755).
#
# Revision 1.71  2014/03/03 06:35:06  palin2
# -Initial check-in ShrinkRay host side Diag.
#
# Revision 1.70  2014/02/18 09:11:11  alpeng
# CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
#
# Revision 1.69.2.3  2014/04/30 13:47:20  kodko
# Support 1548P/2222P clock/trigger in/out verification.
#
# Revision 1.69.2.2  2014/02/24 09:02:43  kodko
# Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
#
# Revision 1.69.2.1  2013/12/25 09:03:06  kodko
# Initial check-in for NGVM  Timing Card.
#
# Revision 1.69  2013/12/18 06:32:59  hroni
# use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64
#
# Revision 1.68  2013/12/16 01:29:36  alpeng
# update pciutils version
#
# Revision 1.67  2013/11/27 08:56:37  hroni
# check for valid build server
#
# Revision 1.66  2013/11/27 07:10:20  mcharon
# change path of mkid32 option from 3.2d-p2 to 3.2d-p4...to fix make depend problem
#
# Revision 1.65  2013/11/11 21:27:39  mcharon
# add files print_hex.[ch] to makefile
#
# Revision 1.64  2013/10/22 14:32:18  danchung
# Add support for PLX PCIe switch
#
# Revision 1.63  2013/10/16 02:18:03  alpeng
# support pci_get_device() from pciutils package
#
# Revision 1.62  2013/10/08 11:14:27  erwu2
# enhanced err msg first check-in
#
# Revision 1.61  2013/10/08 08:48:26  tirawan
# Woodlawn collapsed to main trunk
#
# Revision 1.60  2013/09/06 22:52:22  ptong
# Make cavecreek_sgmii.c sharable among Overlod and Utah
#
# Revision 1.59  2013/07/22 19:37:03  mcharon
# move hts to utah dir/add platform_stub
#
# Revision 1.58  2013/07/16 10:06:16  alpeng
# put platform_slot.c for general using
#
# Revision 1.57  2013/06/04 07:09:15  hroni
# move platform specific files to the corresponding directory (i.e. x86 and utah)
#
# Revision 1.56  2013/05/31 12:43:16  danchung
# Porting PSU source code from Nightster for Juno.
#
# Revision 1.55  2013/05/14 00:13:06  mcharon
# don't delenet dpependancies and nettarget file in make clean
#
# Revision 1.54  2013/05/09 19:25:20  mcharon
# remove unused header files. fixed dependancy compile problem
#
# Revision 1.53  2013/05/09 05:50:37  alpeng
# move overlord common codes from overlord/x86 to overlord
#
# Revision 1.52  2013/05/02 17:29:13  mcharon
# remove file
#
# Revision 1.51  2013/03/31 05:03:07  iachang
# Support SM Lebowski on Overlord
#
# Revision 1.50  2013/03/05 02:11:26  liwwang
# add ngwic prince support
#
# Revision 1.49  2013/02/05 23:55:55  ptong
# Replace BCM lib files list with wildcard command
#
# Revision 1.48  2013/01/31 10:48:45  alpeng
# supported CLI cmds for voltage and freq margin
#
# Revision 1.47  2013/01/14 18:34:33  mcharon
# remove static flag to remove compile warning for tftpclient.c
#
# Revision 1.46  2012/12/20 05:42:01  mcharon
# bring in source for tftp client
#
# Revision 1.45  2012/11/17 02:25:24  palin2
# Add PLL test support.
#
# Revision 1.44  2012/11/16 23:55:17  ptong
# Change the path to BCM lib
#
# Revision 1.43  2012/11/14 01:47:33  ptong
# Move BCM and CVMX vendor src tree to /auto/overlord/ovld-vendor-src
#
# Revision 1.42  2012/11/07 23:57:56  mcharon
# remove platform_124mux.c from makefile to fix make depend issue
#
# Revision 1.41  2012/11/07 14:01:01  alpeng
# fixed compiler warning
#
# Revision 1.40  2012/11/07 11:01:31  alpeng
# update Makefile for previois check in
#
# Revision 1.39  2012/11/06 22:20:06  huanngo
# Remove tcpip.h and tcpip.c which are not used
#
# Revision 1.38  2012/11/06 20:47:52  mcharon
# remove linux_term*
#
# Revision 1.37  2012/11/06 20:43:13  mcharon
# remove linux_platform
#
# Revision 1.36  2012/11/06 20:39:50  mcharon
# add headers/cleanup/remove unneeded functions/files
#
# Revision 1.35  2012/10/17 10:39:34  alpeng
# first check in to support overdrive
#
# Revision 1.34  2012/09/27 22:04:04  mcharon
# don't bundle fpga into image; support dynamic fpga download
#
# Revision 1.33  2012/09/19 07:29:02  palin2
# 1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
#    and related debug utilities support in Overlord Diag.
# 2. Add "PCIe 10-bit PRBS Master External Loopback test" and
#    related debug utilities support at NGSM TestCard side.
#
# Revision 1.32  2012/08/24 23:26:37  mcharon
# back out of previous commit; do not let make clean remove dependancies
#
# Revision 1.31  2012/08/22 19:30:40  mcharon
# remove cs_key_buf.h file
#
# Revision 1.30  2012/08/22 19:14:30  mcharon
# let make clean remove dependancies file
#
# Revision 1.29  2012/08/22 16:32:50  palin2
# First check-in to supprt XAUI test on TestCard.
#
# Revision 1.28  2012/08/20 20:17:43  ptong
# Change TESTCARD_OBJS to TESTCARD_BASE
#
# Revision 1.27  2012/08/14 11:30:56  palin2
# Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.
#
# Revision 1.26  2012/08/08 22:19:41  palin2
# 1. Move TestCard UART external loopback test to "testcard_uart.c".
# 2. Add support TestCard UART internal loopback test and related utilities.
#
# Revision 1.25  2012/08/08 00:04:28  mcharon
# add versino
#
# Revision 1.24  2012/08/07 23:38:33  mcharon
# run banner.sh during compile to support versioning
#
# Revision 1.23  2012/07/31 17:08:20  palin2
# Initial check-in for TestCard PCIe tests.
#
# Revision 1.22  2012/07/30 15:47:09  palin2
# Add support TestCard UART loopback test.
#
# Revision 1.21  2012/07/25 06:39:42  palin2
# Help Danny(danchung) to check-in his changes for TestCard UART loopback test.
#
# Revision 1.20  2012/07/24 23:57:59  ptong
# Bump up release string to 2.0.0 and add -Werror in Makefile
#
# Revision 1.19  2012/07/23 17:33:54  palin2
# Initial check-in for Overlord Test Card diag tests.
#
# Revision 1.18  2012/07/19 18:36:42  mcharon
# remove dc3_cookie files
#
# Revision 1.17  2012/07/16 00:26:10  palin2
# Put "touch dependancies" back because we need it, and this is not the
# root cause for x86 side daily build clock skew issue.
#
# Revision 1.16  2012/06/25 23:33:46  mcharon
# support programming cavecreek eeprom
#
# Revision 1.15  2012/06/22 01:20:06  mcharon
# support volate monitor firmware download
#
# Revision 1.14  2012/06/15 18:20:37  kwochan
# When `make clean`, keep dependancies file
#
# Revision 1.13  2012/06/15 00:50:54  palin2
# To fix x86 clock skew compiler warning.
#
# Revision 1.12  2012/06/01 22:28:25  mcharon
# add api for uart interface test
#
# Revision 1.11  2012/05/31 22:01:23  palin2
# Turn on "-Wall" compiler flag.
#
# Revision 1.10  2012/05/30 03:13:45  palin2
# Fix x86 daily build issue.
#
# Revision 1.9  2012/05/16 07:52:06  srane
# Add Graffham.
#
# Revision 1.8  2012/05/10 09:51:21  palin2
# To change Image name and add .h files make dependency support.
#
# Revision 1.7  2012/05/04 20:01:46  mcharon
# use void* instead of int as argument to to func ptrs in ngio_intf
#
# Revision 1.6  2012/05/02 22:00:05  mcharon
# add api to support tftp file from remote to local
#
# Revision 1.5  2012/04/11 21:27:16  ptong
# Setup cavium named block for mailbox area, and use nc server on cavium to take command from host
#
# Revision 1.4  2012/03/29 19:36:28  ksabzwar
# Added canis source code path
#
# Revision 1.3  2012/03/29 18:46:47  ksabzwar
# Initial check in into ng_diag
#
# Revision 1.2  2012/03/28 23:35:54  huanngo
# Remove the firmware files for Patriot
#
# Revision 1.1.1.1  2012/03/23 23:02:07  ptong
# Initial archive of ng_diag module
#
# Revision 1.1.2.54  2012/03/05 17:52:49  ywen
# Add support for Fortitude.
#
# Revision 1.1.2.53  2012/02/29 20:44:29  palin2
# Add to support to show pressure and internal temp info of Barometer.
#
# Revision 1.1.2.52  2012/02/24 20:54:18  mcharon
# support act1 lib
#
# Revision 1.1.2.51  2012/02/24 20:53:32  mcharon
# add libs/remove stubs
#
# Revision 1.1.2.50  2012/02/24 17:30:07  palin2
# Initial check-in for 12V PoE PSU I2C device utilities.
#
# Revision 1.1.2.49  2012/02/14 21:00:53  ksabzwar
# Added Canis SM card entry function for sanity test on Overlord
#
# Revision 1.1.2.48  2012/01/30 18:56:33  ptong
# Fix eth_pkt_tx and rx api for patriot bringup
#
# Revision 1.1.2.47  2012/01/29 03:36:14  mcharon
# wic/sm use different quack addr
#
# Revision 1.1.2.46  2012/01/20 19:00:45  mcharon
# fix make depend problem
#
# Revision 1.1.2.45  2012/01/20 06:10:09  mcharon
# add pca.c pca.o
#
# Revision 1.1.2.44  2012/01/13 01:38:12  huanngo
# Support SM Patriot on Overlord
#
# Revision 1.1.2.43  2012/01/09 17:01:23  palin2
# Initial check-in for psu tests.
#
# Revision 1.1.2.42  2011/12/29 05:27:50  mcharon
# support ngio/support isr request_irq in user space
#
# Revision 1.1.2.41  2011/12/08 03:05:40  mcharon
# support interrupt
#
# Revision 1.1.2.40  2011/12/05 01:16:15  ptong
# Add test for cavecreek sgmii ports
#
# Revision 1.1.2.39  2011/11/24 00:52:32  mcharon
# add debug routines
#
# Revision 1.1.2.38  2011/11/17 08:17:08  alpeng
# support  act2 lite
#
# Revision 1.1.2.37  2011/11/09 06:49:49  alpeng
# first check in, support eeprom access
#
# Revision 1.1.2.36  2011/11/08 12:31:56  palin2
# Added DIMM utilities support on Overlord x86.
#
# Revision 1.1.2.35  2011/11/02 22:13:20  ptong
# Just use make. now need to use PLATFORM=overlord to compile overlord image
#
# Revision 1.1.2.34  2011/10/28 23:18:04  mcharon
# fix code for 1.3 fpga
#
# Revision 1.1.2.33  2011/10/26 20:31:02  ptong
# Added BCM GE swtich tests
#
# Revision 1.1.2.32  2011/10/21 20:17:17  mcharon
# *** empty log message ***
#
# Revision 1.1.2.31  2011/10/11 18:13:39  mcharon
# support reset/unreset
#
#
# $Endlog$
