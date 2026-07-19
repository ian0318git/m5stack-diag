#$Id: Makefile,v 1.2 2014/06/03 10:53:29 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/Makefile,v $
#------------------------------------------------------------------------------
#Makefile - Makefile to build c executable in lebowski_example module side
#           for python menu
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
default: all

#
# Tool Chain
#
CC = gcc
PY = python2.7

#
# module folder and tgz name
#
MOD_TARGET = leb_python_example

#
# The SRG environment common source directory
#
IROOT              = ../../../..
INC_DIR            = ../../../include
# tar cmd needs related path which started from current folder
SRC_DIR            = ../..

#
# The python-specific source directory
#
PY_BIN         = $(IROOT)/../py_interpreter_v2_7_3/py_interpreter
PY_DIR         = $(PY_BIN)/include/$(PY)
PY_SRC_DIR     = $(SRC_DIR)/Python
PY_INC_DIR     = $(PY_SRC_DIR)/include
PY_SCRIPT_DIR  = $(PY_SRC_DIR)/script
PY_COM_SRC_DIR = $(PY_SRC_DIR)/src
PY_CFG_DIR     = $(PY_SRC_DIR)/config

#
# Include paths for compiler
#
INC	= \
	-I$(PY_DIR) \
	-I$(PY_INC_DIR) \
	-I$(PY_SRC_DIR) \
	-I$(INC_DIR)

#
# GCC FLAGS
#
LDFLAGS            = -L$(PY_BIN)/lib -l$(PY) -lutil -ldl -lnsl -lpthread -lm \
-lrt -Wl,--relax

#
# python-specific source files
#
PY_BASE = \
	$(PY_COM_SRC_DIR)/python_error.c \
	$(PY_COM_SRC_DIR)/python_util.c

PY_DIAG_PCFG = \
	$(PY_CFG_DIR)/diag_flag.pcfg

PY_DIAG_HFILE = \
	$(PY_INC_DIR)/diag_flag_create_from_py.h

#
# SRG environment Source files
#
MOD_BASE = \
        leb_mod_fru.c

NET_BASE = \
        $(PY_BASE) \
        $(MOD_BASE)


############## modify the lines below for lebowski module's side test #########
#
# The SRG platform specific source directory
#
LEB_MOD_DIR = ../module

MOD_BASE += \
	$(LEB_MOD_DIR)/leb_mod_util.c

#
# Include paths for compiler
#
INC	+= \
	-I$(LEB_MOD_DIR)

#
# lebowski module side source code example
#
LEB_MOD_INIT = \
        leb_mod_init.c

LEB_MOD_HOST_COMM = \
        leb_mod_host_comm.c

LEB_MOD_MIC_MEM = \
        leb_mod_mic_mem.c

LEB_MOD_MIC_REG = \
        leb_mod_mic_reg.c

LEB_MOD_MIC_RIN = \
        leb_mod_mic_ring.c

LEB_MOD_SMART_COOKIE = \
        leb_mod_smart_cookie_echo.c

#
# lebowski module side obj file example
#
OBJ_HOST_COMM    = leb_mod_host_comm
OBJ_MIC_MEM      = leb_mod_mic_mem
OBJ_MIC_REG      = leb_mod_mic_reg
OBJ_MIC_RIN      = leb_mod_mic_ring
OBJ_SMART_COOKIE = leb_mod_smart_cookie_echo
OBJ_MOD_INIT     = leb_mod_init

OBJ_LEB_MOD = \
        $(OBJ_HOST_COMM) \
        $(OBJ_MIC_MEM) \
        $(OBJ_MIC_REG) \
        $(OBJ_MIC_RIN) \
        $(OBJ_SMART_COOKIE) \
        $(OBJ_MOD_INIT)

#
# lebowski module make rule example
#
$(OBJ_MOD_INIT)    :$(LEB_MOD_INIT)
$(OBJ_HOST_COMM)   :$(LEB_MOD_HOST_COMM)
$(OBJ_MIC_MEM)     :$(LEB_MOD_MIC_MEM)
$(OBJ_MIC_REG)     :$(LEB_MOD_MIC_REG)
$(OBJ_MIC_RIN)     :$(LEB_MOD_MIC_RIN)
$(OBJ_SMART_COOKIE):$(LEB_MOD_SMART_COOKIE)
$(OBJ_LEB_MOD):
	$(CC) $(NET_BASE) $^ -o $@ $(INC) $(LDFLAGS)


all:$(PY_DIAG_HFILE) $(OBJ_LEB_MOD)
#tar lebowski module folder
	tar zcvf $(MOD_TARGET).tgz --exclude=*.pyc --exclude=*.c \
	--exclude=*.h --exclude=create_diag_flag_to_C.py --exclude=Makefile \
	--exclude=leb_py_tftp.sh --exclude=CVS --exclude=*.tgz \
	$(PY_SCRIPT_DIR) $(PY_CFG_DIR) $(LEB_MOD_DIR)

$(PY_DIAG_HFILE):$(PY_DIAG_PCFG)
#generate the diag_flag_create_from_py.h by executing following python cmd
#note that the arg[1] after create_diag_flag_to_C.py is diag_flag.pcfg path
#arg[2] is diag_flag_create_from_py.h path
#$@ : Target name
#$^ : All dependencies/prerequisites
	$(PY_BIN)/bin/python $(PY_SCRIPT_DIR)/create_diag_flag_to_C.py $^ $@

clean:
	rm -f $(OBJ_LEB_MOD) $(MOD_TARGET).tgz

.PHONY: all clean

#**********History**********
#$Log: Makefile,v $
#Revision 1.2  2014/06/03 10:53:29  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.6  2014/04/29 11:40:38  erwu2
#update python file structure
#
#Revision 1.1.2.5  2014/04/24 08:53:51  erwu2
#merge makefile and add flag example to test
#
#Revision 1.1.2.4  2014/04/19 07:14:34  erwu2
#modularized makefile updated
#
#Revision 1.1.2.3  2014/01/20 05:53:36  erwu2
#improve executable building
#
#Revision 1.1.2.2  2014/01/16 11:00:09  erwu2
#update python files
#
#Revision 1.1.2.1  2013/12/19 10:25:19  erwu2
#improve tftp dnld process
#
#
#
#$Endlog$
