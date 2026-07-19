# $Id: Makefile,v 1.5 2020/05/25 09:37:06 qingcwan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/Makefile,v $
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

NANOOKDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
ENV_SRC     = $(DEV_DRV)/dev_csco_10698
MRV_88E1543_SRC = $(DEV_DRV)/dev_88e1543_marvell
MRV_88E1680_SRC = $(DEV_DRV)/dev_88e1680_marvell
MRV_98DXC323_SRC = $(DEV_DRV)/dev_98dxc323_marvell
NXP_LM75B_SRC   = $(DEV_DRV)/dev_nxp_lm75b
PCA9557_SRC     = $(DEV_DRV)/dev_pca9557
PCA9555_SRC     = $(DEV_DRV)/dev_pca9555
TMPX75_SRC      = $(DEV_DRV)/dev_tmpx75
PLUG_COMMON_SRC = $(COMSRCDIR)/plug_common
PLUG_TC_SRC     = $(COMSRCDIR)/plug_testcard
PLUG_LTE_SRC    = $(COMSRCDIR)/plug_lte
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel


# TAM lib. request : ENGTAM-1223
#TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/ISR4K_2099_SUDI
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/isr9k_curie1ru/aikido
#TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/victory
DREAMLINER_SRC_PATH  = $(COMSRCDIR)/dreamliner_cpss41
#DREAMLINER_PHY_PATH  = $(COMSRCDIR)/dreamliner_cpss41/phy
#REAMLINER_CPSS_PATH  = $(COMSRCDIR)/dreamliner_cpss41/cpss
DREAMLINER_PLD_PATH = /auto/sp-engops/diags/pld/dreamliner_cpss41
DREAMLINER_CPSS_PATH  = $(DREAMLINER_PLD_PATH)/cpss
DREAMLINER_PHY_PATH  = $(DREAMLINER_PLD_PATH)/phy


#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(NANOOKDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(PCA9557_SRC) \
	-I$(PCA9555_SRC) \
	-I$(TMPX75_SRC) \
	-I$(MRV_88E1543_SRC) \
	-I$(MRV_88E1680_SRC) \
	-I$(MRV_98DXC323_SRC) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(ENV_SRC) \
	-I$(EEPROM_SRC) \
	

INC     += \
	-I$(DREAMLINER_SRC_PATH) \
	-I$(DREAMLINER_PHY_PATH) \
        -I$(DREAMLINER_CPSS_PATH)/include/common \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/config \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/config/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/cos \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/cpssHwInit \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/cpssHwInit/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/diag \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/diag/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/init \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/init/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/networkIf \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/phy \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/phy/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/port \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/port/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/smi \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/systemRecovery \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/common/systemRecovery/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/driver \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/driver/interrupts \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDrv \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDrv/drivers \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os/gtOs \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os/gtStack \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/trace \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/log \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/version \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/log \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/log/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/config \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/config/generic \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/hardware \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/hardware/private \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/interrupts \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/interrupts/dxExMx \
        -I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssDriver/pp/interrupts/generic \
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
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/log \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/log/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/lpm \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/lpm/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/nst \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/nst/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/oam \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/oam/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/policer \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/macCtrl \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PortMapping \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/hw \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/ram \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/lpm/tcam \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/protection \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/protection/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tcam \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tcam/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tmGlue \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tmGlue/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/virtualTcam \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/virtualTcam/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cos \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cos/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cscd \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cscd/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/extMac \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/extMac/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/i2c \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/i2c/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/ip \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/ip/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/log \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/mirror \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/mirror/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/nst \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/nst/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/pcl \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/pcl/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/policer \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/policer/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/private/utils \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/smi \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/smi/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/srvCpu \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/srvCpu/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tm \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tm/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tunnel \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tunnel/private \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/version \
        -I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/version/private \


#
# defines for compiler
#

DEFS = \
	-DNANOOK \
	-DLINUX_APP \
	-DTFTP_CLIENT  \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DAIKIDO_ACT2 \
    -DCPU_LE \
	-DAIKIDO_SUPPORT_AIK \
        -DAIKIDO_WRITE_UTIL \
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

DREAMLINER_DEFS = \
	-DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS


CPPOPTS         = -Wall -Werror -Wno-sizeof-pointer-memaccess -Wno-unused-but-set-variable -Wformat=0  $(INC) ${DEFS} $(DREAMLINER_DEFS)
LDFLAGS         = -lpthread -lm
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into NANOOK project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJNAME = $(TAMLIB)/tam_lib_objname.a


LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= nanooklnx

#
# Common device driver objects
#
DEV_OBJS = \
		   dev_object.o \
		   dev_nxp_lm75b.o \
		   dev_ltc4215.o \
		   dev_pca9557.o \
		   dev_pca9555.o \
		   dev_tmpx75.o \
		   dev_88e1543.o \
		   dev_88e1680.o \
		   dev_98dxc323.o \
		   dev_csco_10698.o


DEV_BASE = \
		   $(DEV_DRV)/dev_object.c \
		   $(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
		   $(MRV_88E1543_SRC)/dev_88e1543.c \
		   $(MRV_88E1680_SRC)/dev_88e1680.c \
		   $(MRV_98DXC323_SRC)/dev_98dxc323.c \
		   $(PCA9557_SRC)/dev_pca9557.c \
		   $(PCA9555_SRC)/dev_pca9555.c \
		   $(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		   $(TMPX75_SRC)/dev_tmpx75.c \
		   $(ENV_SRC)/dev_csco_10698.c \
		   $(EEPROM_SRC)/dev_at24c0n.c \


DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
	$(PCA9557_SRC)/dev_pca9557.h \
	$(PCA9555_SRC)/dev_pca9555.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(TMPX75_SRC)/dev_tmpx75.h \
	$(MRV_88E1543_SRC)/dev_88e1543.h \
	$(MRV_88E1680_SRC)/dev_88e1680.h \
	$(MRV_98DXC323_SRC)/dev_98dxc323.h \
	$(ENV_SRC)/dev_csco_10698.h \
	$(EEPROM_SRC)/dev_at24c0n.h \


FORTITUDE_OBJS = ngwic_fortitude.o
FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c


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
	nim_dm_cpss_trace.o 

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
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_trace.c 

DREAMLINER_LIB = \
	$(DREAMLINER_PLD_PATH)/libdreamlinersdk.a


PRINCE_OBJS = ngwic_prince.o
PRINCE_BASE = $(COMSRCDIR)/ngwic_prince.c

REVA_OBJS = ngwic_reva.o
REVA_BASE = $(COMSRCDIR)/ngwic_reva.c

ARKENSTONE_OBJS = nim_arkenstone.o
ARKENSTONE_BASE = $(COMSRCDIR)/nim_arkenstone.c

WALLANDER_OBJS = nim_wallander.o
WALLANDER_BASE = $(COMSRCDIR)/nim_wallander.c

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
    linux_uart.o \
    linux_pciutils.o \
    linux_usb_test.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    adapter_fpga.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	linux_ntwk.o \
	pca.o \
	linux_coretest.o \
	oir_ltc4215_api.o \
	uio_utils.o \
        linux_block_test.o \

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
    $(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/linux_uart.c \
    $(COMSRCDIR)/linux_pciutils.c \
    $(COMSRCDIR)/linux_usb_test.c \
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
    $(COMSRCDIR)/slot.c \
    $(COMSRCDIR)/adapter_fpga.c \
    $(COMSRCDIR)/tam_aikido_upgrade.c \
    $(COMSRCDIR)/uio_utils.c \
    $(COMSRCDIR)/linux_block_test.c \

COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_pciutils.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/linux_ntwk.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h \
	$(INCDIR)/cross_platform.h \
        $(INCDIR)/ethernet.h \
        $(INCDIR)/adapter_fpga.h \
	$(INCDIR)/uio_utils.h \
        $(INCDIR)/linux_block_test.h \




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
	diag_gephy_1543_test.o \
	diag_gephy_1543_lib.o \
        diag_gephy_1543_util.o \
        diag_esw_test.o \
        diag_esw_lib.o \
        diag_esw_util.o \
	diag_cpld_lib.o \
	diag_led_test.o \
	diag_led_util.o \
        diag_eth_pkt_txrx.o \
        diag_eth_pkt_txrx_api.o \
        diag_eth_pkt_txrx_utils.o \
	diag_ddr4_lib.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	i2c_api.o \
	diag_fpga_i2c_lib.o \
	diag_usb_lib.o \
	diag_usb_util.o \
	diag_usb_test.o \
	diag_temp_snsr_test.o \
	diag_cpu_util.o \
	platform_stub.o \
	platform_cookie.o \
	platform_tam_cookie.o \
	platform_fan.o \
	legacy_smart_cookie.o \
	tam_act2_api_drv_support.o \
	tam_aikido_upgrade.o \
	diag_emmc_test.o \
	diag_emmc_util.o \
	dnv_gpio_lib.o \
	diag_uart_lib.o \
	dnv_eth_lib.o \
	diag_rtc_lib.o \
	diag_rtc_test.o \
	diag_rtc_util.o \
	platform_cpu.o \
	nanook_comm.o \
	full_load_util.o \
	ngio.o \
	slot.o \
	platform_slot.o \
	$(PLUG_OBJS) \
	diag_m2_test.o \
	platform_pwr_seq.o \
	platform_spi.o \
	dash_fpga.o \
        dash_reg_test.o \
        dash_get_addr.o \
        dash_mem.o \
        platform_prom.o \
        diag_fpga_util.o \
	diag_async_test.o \
	crocus_scc_diag.o \
	diag_press_sensor_test.o \
	platform_intr_test.o \


PLAT_BASE = \
	$(NANOOKDIR)/linux_main.c \
	$(NANOOKDIR)/diag.c \
	$(NANOOKDIR)/mb_tests.c \
	$(NANOOKDIR)/diag_boot_flash_test.c \
	$(NANOOKDIR)/diag_spi_flash_util.c \
	$(NANOOKDIR)/diag_gephy_1543_test.c \
	$(NANOOKDIR)/diag_gephy_1543_lib.c \
        $(NANOOKDIR)/diag_gephy_1543_util.c \
	$(NANOOKDIR)/diag_esw_test.c \
	$(NANOOKDIR)/diag_esw_lib.c \
	$(NANOOKDIR)/diag_esw_util.c \
	$(NANOOKDIR)/diag_cpld_lib.c \
	$(NANOOKDIR)/diag_led_test.c \
	$(NANOOKDIR)/diag_led_util.c \
	$(NANOOKDIR)/diag_eth_pkt_txrx.c \
	$(NANOOKDIR)/diag_eth_pkt_txrx_api.c \
	$(NANOOKDIR)/diag_eth_pkt_txrx_utils.c \
	$(NANOOKDIR)/diag_ddr4_lib.c \
	$(NANOOKDIR)/diag_i2c_lib.c \
	$(NANOOKDIR)/diag_i2c_test.c \
	$(NANOOKDIR)/i2c_api.c \
	$(NANOOKDIR)/diag_fpga_i2c_lib.c \
	$(NANOOKDIR)/diag_usb_lib.c \
	$(NANOOKDIR)/diag_usb_util.c \
	$(NANOOKDIR)/diag_usb_test.c \
	$(NANOOKDIR)/diag_temp_snsr_test.c \
	$(NANOOKDIR)/diag_cpu_util.c \
	$(NANOOKDIR)/platform_stub.c \
	$(NANOOKDIR)/platform_cookie.c \
	$(NANOOKDIR)/platform_tam_cookie.c \
	$(NANOOKDIR)/platform_fan.c \
	$(NANOOKDIR)/legacy_smart_cookie.c \
	$(NANOOKDIR)/tam_act2_api_drv_support.c \
	$(NANOOKDIR)/diag_emmc_test.c \
	$(NANOOKDIR)/diag_emmc_util.c \
	$(NANOOKDIR)/dnv_gpio_lib.c \
	$(NANOOKDIR)/diag_uart_lib.c \
	$(NANOOKDIR)/dnv_eth_lib.c \
	$(NANOOKDIR)/diag_rtc_lib.c \
	$(NANOOKDIR)/diag_rtc_test.c \
	$(NANOOKDIR)/diag_rtc_util.c \
	$(NANOOKDIR)/platform_cpu.c \
	$(NANOOKDIR)/nanook_comm.c \
	$(NANOOKDIR)/full_load_util.c \
	$(NANOOKDIR)/ngio.c \
	$(NANOOKDIR)/platform_slot.c \
	$(NANOOKDIR)/diag_m2_test.c \
	$(NANOOKDIR)/platform_pwr_seq.c \
	$(NANOOKDIR)/platform_spi.c \
	$(NANOOKDIR)/dash_fpga.c \
        $(NANOOKDIR)/dash_get_addr.c \
        $(NANOOKDIR)/dash_mem.c \
        $(NANOOKDIR)/dash_reg_test.c \
        $(NANOOKDIR)/platform_prom.c \
        $(NANOOKDIR)/diag_fpga_util.c \
	$(NANOOKDIR)/diag_async_test.c \
	$(NANOOKDIR)/crocus_scc_diag.c \
	$(NANOOKDIR)/diag_press_sensor_test.c \
	$(NANOOKDIR)/platform_intr_test.c \


PLAT_HFILES = \
    $(NANOOKDIR)/diag_common.h \
    $(NANOOKDIR)/platform_i2c.h \
    $(NANOOKDIR)/mb_tests.h \
    $(NANOOKDIR)/diag_eth_info.h \
    $(NANOOKDIR)/diag_eth_pkt_txrx.h \
    $(NANOOKDIR)/diag_eth_pkt_txrx_api.h \
    $(NANOOKDIR)/diag_eth_pkt_txrx_utils.h \
    $(NANOOKDIR)/diag_ddr4_lib.h \
    $(NANOOKDIR)/diag_i2c_lib.h \
    $(NANOOKDIR)/diag_i2c_test.h \
    $(NANOOKDIR)/diag_i2c_addr.h \
    $(NANOOKDIR)/diag_boot_flash_test.h \
    $(NANOOKDIR)/diag_spi_flash_util.h \
    $(NANOOKDIR)/diag_fpga_i2c_lib.h \
    $(NANOOKDIR)/diag_storage_lib.h \
    $(NANOOKDIR)/diag_usb_lib.h \
    $(NANOOKDIR)/diag_usb_util.h \
    $(NANOOKDIR)/diag_usb_test.h \
    $(NANOOKDIR)/diag_temp_snsr_test.h \
	$(NANOOKDIR)/diag_gephy_1543_test.h \
	$(NANOOKDIR)/diag_gephy_1543_lib.h \
	$(NANOOKDIR)/diag_gephy_1543_util.h \
    $(NANOOKDIR)/diag_esw_test.h \
    $(NANOOKDIR)/diag_esw_lib.h \
    $(NANOOKDIR)/diag_esw_util.h \
    $(NANOOKDIR)/diag_cpu_util.h \
    $(NANOOKDIR)/platform_stub.h \
    $(NANOOKDIR)/platform_cookie.h \
    $(NANOOKDIR)/platform_tam_cookie.h \
    $(NANOOKDIR)/legacy_smart_cookie.h \
    $(NANOOKDIR)/tam_act2_api_drv_support.h \
	$(NANOOKDIR)/diag_emmc_test.h \
	$(NANOOKDIR)/diag_emmc_util.h \
    $(NANOOKDIR)/dnv_gpio_lib.h \
    $(NANOOKDIR)/diag_uart_lib.h \
    $(NANOOKDIR)/dnv_eth_lib.h \
    $(NANOOKDIR)/diag_rtc_lib.h \
    $(NANOOKDIR)/diag_rtc_test.h \
    $(NANOOKDIR)/diag_rtc_util.h \
    $(NANOOKDIR)/platform_cpu.h \
    $(NANOOKDIR)/nanook_comm.h \
    $(NANOOKDIR)/full_load_util.h \
	$(NANOOKDIR)/ngio.h \
	$(NANOOKDIR)/platform_slot.h \
	$(NANOOKDIR)/diag_m2_test.h \
	$(NANOOKDIR)/platform_pwr_seq.h \
	$(NANOOKDIR)/dash_fpga.h \
        $(NANOOKDIR)/uart_fpga.h \
        $(NANOOKDIR)/platform_prom.h \
        $(NANOOKDIR)/diag_fpga_util.h \
	$(NANOOKDIR)/diag_async_test.h \
	$(NANOOKDIR)/nanook_crocus_def.h \
	$(NANOOKDIR)/crocus_reg.h \
	$(NANOOKDIR)/diag_press_sensor_test.h \
	$(NANOOKDIR)/platform_intr_test.h \

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
        $(NANOOKDIR)/ngio_testcard.c \
        $(NANOOKDIR)/testcard_fpga.c \
        $(NANOOKDIR)/testcard_eth.c \
        $(NANOOKDIR)/testcard_xaui.c \
        $(NANOOKDIR)/testcard_pcie.c \
        $(NANOOKDIR)/testcard_plx_pcie_sw.c \
        $(NANOOKDIR)/testcard_tlk_10232.c \
        $(NANOOKDIR)/testcard_uart.c

TESTCARD_HFILES = \
        $(NANOOKDIR)/ngio_testcard.h \
        $(NANOOKDIR)/testcard_fpga.h \
        $(NANOOKDIR)/testcard_eth.h \
        $(NANOOKDIR)/testcard_plx_pcie_sw.h \
        $(NANOOKDIR)/testcard_tlk_10232.h \
        $(NANOOKDIR)/testcard_xaui.h \
        $(NANOOKDIR)/testcard_pcie.h



#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
    $(PLAT_OBJS) \
    $(COM_OBJS) \
    $(DEV_OBJS) \
    $(LIB_PCI_UTILS) \
    $(LIB_TAM_ACT2_MFG) \
    $(LIB_TAM_ACT2) \
    $(LIB_TAM_OBJNAME) \
    $(TESTCARD_OBJS) \
    $(FORTITUDE_OBJS) \
    $(DREAMLINER_OBJS) \
    $(DREAMLINER_LIB) \
    $(REVA_OBJS) \
    $(ARKENSTONE_OBJS) \
    $(WALLANDER_OBJS) \
    $(PRINCE_OBJS)

#
#########   CBASE #########
#

CBASE	= \
    $(COM_BASE) \
    $(DEV_BASE) \
    $(PLAT_BASE) \
    $(TESTCARD_BASE) \
    $(FORTITUDE_BASE) \
    $(DREAMLINER_BASE) \
    $(REVA_BASE) \
    $(ARKENSTONE_BASE) \
    $(WALLANDER_BASE) \
    $(PRINCE_BASE)

#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
        $(TESTCARD_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(NANOOKDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(RTC_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(NXP_LM75B_SRC): \
	$(MRV_88E1543_SRC): \
	$(MRV_88E1680_SRC): \
	$(MRV_98DXC323_SRC): \
	$(LTE_SWI_SRC): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PLD_PATH): \
	$(TAMLIB)

vpath %.h \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(NANOOKDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
    $(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(MRV_88E1543_SRC): \
	$(MRV_88E1680_SRC): \
	$(MRV_98DXC323_SRC): \
    $(LTE_SWI_SRC): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PHY_PATH): \
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
	@echo "NANOOK_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "NANOOKDIR: " $(NANOOKDIR)
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
	sh $(IROOT)/utils/banner.sh NANOOK_INTEL
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
# Revision 1.5  2020/05/25 09:37:06  qingcwan
# CSCvu31200:Fix bug introduced by prev commit to merge in switzer-carrier code.
#
# Revision 1.4  2020/04/20 02:28:24  lucywang
#
# 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
# 2. Added to support NIM Prince
# 3. (CSCvn43011) add retry workaround for Deverton issue
# 4. add debug message and set default value to seneors
# 5. Reverted Register value of temp/press snsr after test
# 6. Bumped up version to 1.0.2
#
# Revision 1.3  2019/12/12 07:57:37  yungchen
# Add define flag to fix diffenence tam library build failed issue
#
# Revision 1.2  2019/12/11 10:10:27  lucywang
# Merged Nanook to main trunk
#
#
# $Endlog$
