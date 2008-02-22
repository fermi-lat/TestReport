#include <string>
#include <fstream>
#include <getopt.h>
#include "RunVerify.h"

int main(int argn, char** argv)
{
  std::string digiRootFile, xmlFileName, histoFileName, optionFileName;
  std::string digiRootFile_def, xmlFileName_def, histoFileName_def;
  int opt;
 
  while ( (opt = getopt(argn, argv, "o:d:x:r:")) != EOF ) {
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
    } 
  }

  if ( argn - optind > 0 ) {
    cout << "RunVerify only takes options, not bare arguments; using default settings." << endl;
  }

  if (optionFileName == "") optionFileName = "../src/VerifyOption.dat";
  std::ifstream optionFile(optionFileName.c_str());
  optionFile >> digiRootFile_def >> xmlFileName_def >> histoFileName_def ;

  if (digiRootFile == "") digiRootFile = digiRootFile_def;
  if (xmlFileName == "") xmlFileName = xmlFileName_def;
  if (histoFileName == "") histoFileName = histoFileName_def;

  RunVerify v(xmlFileName.c_str(), histoFileName.c_str());
  v.analyzeDigi(digiRootFile.c_str());
  v.generateXml();
}

