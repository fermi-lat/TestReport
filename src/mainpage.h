// (Special "header" just for doxygen)

/*! @mainpage  package TestReport

@author Xin Chen

@section intro Introduction

This package contains codes to generate test report for I&T/SVAC. The report is
intended to be generated for every run. The package includes following 
executables/scripts

<ul>
<li> cmt/GlastRelease.env needs to be modified to reflect actual CMT path. 
</li>
<li> TestReport.exe. It read data from a digi root file, analyze it and produce
 a text file containing some plots/tables of test results. It reads 
configuration info from a job
 option file. First line in the option file is name of the digi root file to be
analyzed, second line is name of the recon root file to be analyzed(Even if 
the  recon root files does not exist, the name has to be there and the file 
will be ignored.), third line is the dir where output text is going to be 
produced, 
fourth line is prefix of the output file. For example, if prefix is test, a text
file called test_report.txt will be produced. The exe uses its first argument
 as name of the option file; when there is no argument, it uses 
../src/ReportOption.dat as the option file</li>
<li> ../pl/GenTestReport.pl. It does following things:
  <ul>
  <li>run TestReport.exe to produce doxygen file, eps and gif files. </li>
  <li>run doxygen to produce html and latex files. </li>
  <li>run latex to produce a ps file, then run ps2pdf to produce a pdf file. </li>
  </ul>
  Depending where the TestReport package is installed and the CMT path, some 
variables in GenTestReport.pl need to be changed.
</li>
</ul>

*/
