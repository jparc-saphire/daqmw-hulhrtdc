#!/usr/bin/perl

#use strict;
#use warnings;

my ($startrun,$endrun) = @ARGV;

if ((not defined $startrun)||(not defined $endrun)) {
    die "Usage: batchrunjamtpc.pl startrun endrun\n";
}

#$indir = "/disk1/mrpcdaq/data/dat_E16local2_20200531";
$indir = "/disk1/mrpcdaq/data/dat_E16local_20200526";

print "startrun = $startrun\n";
print "endrun = $endrun\n";

for (my $run=$startrun;$run<=$endrun;$run++) {
    $infile = sprintf("%s/run%06d_0.dat",$indir,$run);
    $outfile = sprintf("%s/run%06d_0.root",$indir,$run);

    #    if ((-e $infile)&&(!(-e $outfile))) {
        if (-e $infile) {
	$command = sprintf("nice /home/mrpcdaq/Analyzer/dat2root.pl %d",$run);
	
	print "$command\n";
	system($command);
	
    }
    
}


