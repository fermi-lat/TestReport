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
my $reconRootFile = <OPTFILE>;
chop($reconRootFile);
my $reportDir = <OPTFILE>;
chop($reportDir);
my $prefix = <OPTFILE>;
chop($prefix);
my $testReportVersion = <OPTFILE>;
chop($testReportVersion);
close(OPTFILE);

my $texFile = 'refman.tex';
my $dviFile = 'refman.dvi';
my $psFile = 'refman.ps';
my $pdfFile = 'refman.pdf';
my $htmlFile = 'index.html';
my $pwd =  $ENV{PWD};

my $appRoot = '/nfs/farm/g/glast/u33/borgland/CodeDev/svac/';
my $testReportVersion = 'v3r4p15';
my $emVersion = 'v6r070329p5';
my $cmtPath = "$appRoot:/nfs/farm/g/glast/u09/builds/rh9_gcc32opt/EngineeringModel/EngineeringModel-$emVersion";
my $cmtDir = "$appRoot/TestReport/$testReportVersion/cmt";
my $exeDir = "$appRoot/TestReport/$testReportVersion/rh9_gcc32opt";
my $doxyFile = "$appRoot/TestReport/$testReportVersion/src/ReportDoxyfile";
my $latexHeader = "$appRoot/TestReport/$testReportVersion/src/latexHeader.tex";

my $shellFile = $reportDir.'/'.$prefix.'_report.scr';
open(SHELLFILE, ">$shellFile") || die "Can't open $shellFile, abortted!";
print SHELLFILE qq{#!/bin/csh \n \n};
print SHELLFILE qq{unsetenv LD_LIBRARY_PATH \n};
print SHELLFILE qq{setenv CMTPATH $cmtPath \n};
print SHELLFILE qq{source $cmtDir/setup.csh \n};
print SHELLFILE qq{$exeDir/TestReport.exe $optionFile \n};
print SHELLFILE qq{cd $reportDir \n};
print SHELLFILE qq{setenv latexHeader '$latexHeader' \n};
print SHELLFILE qq{setenv testReportVersion '$testReportVersion' \n};
print SHELLFILE qq{doxygen $doxyFile \n};
print SHELLFILE qq{mv *.eps latex/ \n};
print SHELLFILE qq{cd latex \n};
print SHELLFILE qq{latex $texFile \n};
print SHELLFILE qq{latex $texFile \n};
print SHELLFILE qq{dvips -o $psFile $dviFile \n};
print SHELLFILE qq{ps2pdf $psFile $pdfFile \n};
print SHELLFILE qq{cd $pwd};
close(SHELLFILE);
system("chmod +rwx $shellFile");
system("$shellFile");
#    system("bsub -q medium -o $dumpFile $shellFile");
