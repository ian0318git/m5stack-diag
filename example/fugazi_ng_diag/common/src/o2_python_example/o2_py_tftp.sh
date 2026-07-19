#$Id: o2_py_tftp.sh,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_py_tftp.sh,v $
#------------------------------------------------------------------------------
#Description: tftp download script for python interpreter binary and
#             o2_pythn_example.
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
#!/bin/bash

if [ ! -d "/tmp/python" ]
then
    mkdir /tmp/python
fi
if [ ! -d "/python_menu" ]
then
    mkdir /python_menu
fi

#tftp download interpreter binary and o2_python_example from TFTPDIR
tftp -g -r $TFTPDIR/py_bin_v2_7_3.tgz -l /tmp/python/py_bin_v2_7_3.tgz \
$TFTP_SERVER
tftp -g -r $TFTPDIR/o2_python_example.tgz -l /tmp/python/o2_python_example.tgz \
$TFTP_SERVER

#decompress tgz files
tar zxvf /tmp/python/py_bin_v2_7_3.tgz -C /
tar zxvf /tmp/python/o2_python_example.tgz -C /python_menu
chmod 777 -R /py_interpreter /python_menu

#remove tgz files under /tmp/python
rm -rf /tmp/python

#**********History**********
#$Log: o2_py_tftp.sh,v $
#Revision 1.2  2014/06/03 10:53:31  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.1  2014/01/16 11:00:09  erwu2
#update python files
#
#Revision 1.1.2.1  2013/12/19 10:25:19  erwu2
#improve tftp dnld process
#
#
#
#$Endlog$
