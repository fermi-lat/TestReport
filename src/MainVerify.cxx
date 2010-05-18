#include <string>
#include <fstream>
#include <getopt.h>
#include "RunVerify.h"

int main(int argn, char** argv)
{
  std::string digiRootFile, xmlFileName, histoFileName, optionFileName;
  std::string digiRootFile_def, xmlFileName_def, histoFileName_def;
  int opt, truncation=0, truncation_def=100;
  bool completeRun = false;
 
  while ( (opt = getopt(argn, argv, "o:d:x:r:t:c")) != EOF ) {
    switch (opt) {
      case 'o':   // Option file name
        optionFileName = string(optarg);
        break;
      case 'd':   // digi file name      
        digiRootFile = string(optarg);
        break;
      case 'x':   // xml output file name      
        xmlFileName = string(optarg);
        break;
      case 'r':   // root output file name      
        histoFileName = string(optarg);
        break;
      case 't':   // truncation     
        truncation = atoi(optarg);
        break;
      case 'c':   // end of run     
        completeRun = true;
        break;
    } 
  }

  if ( argn - optind > 0 ) {
    cout << "RunVerify only takes options, not bare arguments; using default settings." << endl;
  }

  if (optionFileName == "") optionFileName = "../src/VerifyOption.dat";
  std::ifstream optionFile(optionFileName.c_str());
  optionFile >> digiRootFile_def >> xmlFileName_def >> histoFileName_def;

  if (digiRootFile == "") digiRootFile = digiRootFile_def;
  if (xmlFileName == "") xmlFileName = xmlFileName_def;
  if (histoFileName == "") histoFileName = histoFileName_def;
  if (truncation == 0) truncation = truncation_def;

  RunVerify v(histoFileName.c_str());
  int analyze = v.analyzeDigi(digiRootFile.c_str(),completeRun);
  v.writeXmlFile(xmlFileName.c_str(),completeRun,truncation);
  if (analyze==3) return 153;
  return 0;
}

