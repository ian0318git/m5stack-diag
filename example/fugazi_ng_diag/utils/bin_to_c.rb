#!/usr/bin/ruby
# $Id: bin_to_c.rb,v 1.1 2017/08/16 08:13:54 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/utils/bin_to_c.rb,v $
#------------------------------------------------------------------
#
# bin_to_c.rb - Convert bin to h tool.
#
# Copyright (c) 2012-2017 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#
#
$stdin.each_byte {|x|
  printf("0x%02x,\n", x)
}

  
