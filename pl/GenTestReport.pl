#!/usr/local/bin/perl

use strict;

my $numArgs = $#ARGV + 1;

my $optionFile;

if($numArgs == 0) {
    $optionFile = "../src/ReportOption.dat";
}
else {
    $optionFile = $ARGV[0];
}

open(OPTFILE, "<$optionFile") || die "Can't open $optionFile for input, abortted!";

my $digiRootFile = <OPTFILE>;
chop($digiRootFile);
my $reportDir = <OPTFILE>;
chop($reportDir);
my $prefix = <OPTFILE>;
chop($prefix);
my $texFile = $prefix.'_report.txt';
my $dviFile = $prefix.'_report.dvi';
my $psFile = $prefix.'_report.ps';
my $pdfFile = $prefix.'_report.pdf';
my $pwd =  $ENV{PWD};

my $cmtPath = "/nfs/farm/g/glast/u06/chen:/nfs/farm/g/glast/u10/builds/EngineeringModel/EngineeringModel-v1r0401p8";
my $cmtDir = "/nfs/farm/g/glast/u06/chen/TestReport/v0/cmt";
my $exeDir = "/nfs/farm/g/glast/u06/chen/TestReport/v0/rh9_gcc32";

my $shellFile = $reportDir.'/'.$prefix.'_report.scr';
open(SHELLFILE, ">$shellFile") || die "Can't open $shellFile, abortted!";
print SHELLFILE "#!/bin/csh \n \n";
print SHELLFILE qq{unsetenv LD_LIBRARY_PATH \n};
print SHELLFILE qq{setenv CMTPATH $cmtPath \n};
print SHELLFILE qq{source $cmtDir/setup.csh \n};
print SHELLFILE qq{$exeDir/TestReport.exe $optionFile \n};
print SHELLFILE qq{cd $reportDir \n};
print SHELLFILE qq{latex $texFile \n};
print SHELLFILE qq{dvips -o $psFile $dviFile \n};
print SHELLFILE qq{ps2pdf $psFile $pdfFile \n};
print SHELLFILE qq{cd $pwd};
close(SHELLFILE);
system("chmod +rwx $shellFile");
system("$shellFile");
#    system("bsub -q medium -o $dumpFile $shellFile");
