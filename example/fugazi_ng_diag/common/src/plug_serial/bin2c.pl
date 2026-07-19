#! /router/bin/perl5 -w
# $Id: bin2c.pl,v 1.2 2018/08/02 09:35:01 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/bin2c.pl,v $
#------------------------------------------------------------------
# Utility to convert image to array format.
#
#
# Aug 2018, Ian Chang port from /auto/sp-engops/diags/pld/reva directory 
#
# Execute "./bin2c.pl plugser_sb_upgrade.bin plugser_sb_upgrade.c "Ian Chang" prince_xp_ugd"
#
# Copyright (c) 1998~2018 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
#

use strict;
use POSIX qw(strftime);
use Getopt::Std;
use File::Basename;

print STDERR join(' ', $0, @ARGV), "\n";

my $prog = basename($0);

# my %opts = ();

# getopts('su:', \%opts);

use vars qw($opt_s $opt_u);

getopt('su');

# map { printf STDERR "%s => %s\n", $_, $opts{$_}; } keys %opts;

my $use_suffix = $opt_s || '_fw';

my $uncomp_len = $opt_u || 0;

if ($#ARGV != 3) {
    printf STDERR "Usage: %s [ -s ] [ -u bytes ] <input filename> <output filename> <\"contact info\"> <firmware variable name>\n", $prog;
    printf STDERR "\t%s et2.bin et2_firmware \"David Greeson (et2-sw)\" et2_ce1t1_firmware\n", $prog;
    exit(1);
}

my $infn = $ARGV[0];
my $outfn = $ARGV[1];
my $contact = $ARGV[2];
my $prefix = $ARGV[3];

open(IN, "$infn") || die "Can't open $infn: $!\n";

open(OUT, ">$outfn") || die "Can't open $outfn: $!\n";

select((select(OUT), $| = 1)[0]);

my $outroot = basename($outfn, '.c');

my $ncols = 8;

# use strftime() instead...
my @now = localtime();
my $thismon = strftime('%B', @now);
my $thisyear = strftime('%Y', @now);
my $date = strftime('%Y%m%d', @now);

printf OUT <<__EOF__, $outroot, $thismon, $thisyear, $contact, $thisyear;
/*
 *------------------------------------------------------------------
 * %s.c - This is a machine generated file. Do not modify.
 *
 * %s %s, %s
 *
 * Copyright (c) %s by cisco Systems, Inc.
 *------------------------------------------------------------------
 */

__EOF__

printf OUT "\nconst unsigned char %s%s[]", $prefix, $use_suffix;

print OUT " = {\n";

my $data = '';

my $col = 0;

while (read(IN, $data, 1)) {
    print OUT "\t" if ($col == 0);
    printf OUT "0x%02x, ", ord($data);
    print OUT "\n" if ($col == ($ncols - 1));

    $col = ($col + 1) % $ncols;
}

die "$infn: $!\n" if ( $! =~ /\S/ );

close(IN);

print OUT "\n" if ($col != 0);

printf OUT <<__EOF__, $prefix, $prefix, $use_suffix;
};

const unsigned int %s_fw_size = sizeof(%s%s);
__EOF__

if ($uncomp_len) {
    printf OUT "const unsigned int %s_uncomp_fw_size = %d;\n", $prefix, $uncomp_len;
}

print OUT "\n";

close(OUT);

exit(0);

######## HISTORY ########
# $Log: bin2c.pl,v $
# Revision 1.2  2018/08/02 09:35:01  iachang
# Merge Pluggable Serial from branch star-branch-c9xx to main trunk
#
#
# Revision 1.1.2.1  2010/06/23 03:36:44  dgreeson
# New utility to convert a binary file to a hex array
#
# 
#########################
# $Endlog$
#
