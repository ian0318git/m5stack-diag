#$Id: leb_mod_py_menu.sh,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_py_menu.sh,v $
#------------------------------------------------------------------------------
#Description: script for executing python menu in lebowski module side
#             python example
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
#!/bin/bash

cd ../Python/script
/py_interpreter/bin/python diag_menu.py \
/module_python_menu/module/python/leb_diag_export.txt

#**********History**********
#$Log: leb_mod_py_menu.sh,v $
#Revision 1.2  2014/06/03 10:53:30  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.2  2014/04/29 11:40:38  erwu2
#update python file structure
#
#Revision 1.1.2.1  2014/04/24 08:53:50  erwu2
#merge makefile and add flag example to test
#
#
#
#$Endlog$
