#$Id: generic_pcie_lane.sh,v 1.2 2015/05/25 03:59:27 steja Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/utils/generic_pcie_lane.sh,v $
#-----------------------------------------------------------------
#
#generic_pcie_lane.sh - Write the generic PCIe lanes information to a file
#
#July 2013 - Ian Chang porting the code
#
#Copyright (c) 2013-2015 by Cisco Systems, Inc.
#All rights reserved.
#!/bin/sh
if [ -f /skye_pcie_check.txt ]
    then rm /skye_pcie_check.txt
fi

touch /skye_pcie_check.txt

lspci_list_nu=$(lspci | grep 'Class' -c)
for i in `seq 1 $lspci_list_nu`
do
    lspci | grep 'Class' | sed -n "$i"p >> /skye_pcie_check.txt
    lspci -s $(lspci | grep 'Class' | cut -c -8 | sed -n "$i"p) -vvv | grep 'LnkSta:' | cut -c 3- >> /skye_pcie_check.txt
done


#
# Exit from generic_pcie_lane.sh
#
exit 0

#-----------------------------------------------------------------
#$Log: generic_pcie_lane.sh,v $
#Revision 1.2  2015/05/25 03:59:27  steja
#Add Support Skye SM
#
#Revision 1.1.4.2  2015/04/29 11:36:49  steja
#Code check-in to skye-branch2 for ER code review
#
#
#------------------------------------------------------------
#Revision 1.1.2.2  2014/11/27 09:06:24  palin2
#Updated script to support Skye PCIe lanes Scan Test.
#
#Revision 1.1.2.1  2014/07/21 01:56:47  palin2
#Initial check-in Skye module side Diag code.
#
#-----------------------------------------------------------------
#Revision 1.2  2014/02/27 15:01:26  palin2
#Initial check-in ShrinkRay SM side Diag code.
#
#Revision 1.1.4.2  2013/09/13 07:00:15  palin2
#Initial check-in ShrinkRay SM side Diag code.
#
#Revision 1.1.2.1  2013/07/11 15:55:59  iachang
#Add generic PCIe lanes check script.
#
#-----------------------------------------------------------------
#$Endlog$
