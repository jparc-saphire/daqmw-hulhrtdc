#!/usr/bin/perl
my ($run) = @ARGV;

if (not defined $run) {
        die "Usage: dat2root.py run\n";
}

#$datadir = "/disk1/mrpcdaq/data/dat_E16local_20200526";
$datadir = "/disk1/mrpcdaq/data/dat_TSCRPCR_202305";
$anadir = "/home/mrpcdaq/Analyzer";
chdir($anadir);
$command = sprintf("./bin/analysMonitor conf/analyzer.conf.default $datadir/run%06d_0.dat $datadir/run%06d_0.root",$run,$run);

print "$command\n";

system($command);

