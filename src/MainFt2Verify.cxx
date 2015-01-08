#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include "ft2Verify.h"

using std::cout;
using std::endl;

int main(int argn, char** argv)
{
  std::string ft2FileName, xmlFileName, optionFileName;
  std::string ft2FileName_def, xmlFileName_def;
  int opt, truncation=0, truncation_def=100;
 
  while ( (opt = getopt(argn, argv, "o:f:x:t:")) != EOF ) {
    switch (opt) {
      case 'o':   // Option file name
        optionFileName = string(optarg);
        break;
      case 'f':   // ft2 file name      
        ft2FileName = string(optarg);
        break;
      case 'x':   // xml output file name      
        xmlFileName = string(optarg);
        break;
      case 't':   // truncation     
        truncation = atoi(optarg);
        break;
    } 
  }

  if ( argn - optind > 0 ) {
    cout << "ft2Verify only takes options, not bare arguments; using default settings." << endl;
  }

  if (optionFileName == "") optionFileName = "../src/ft2VerifyOption.dat";
  std::ifstream optionFile(optionFileName.c_str());
  optionFile >> ft2FileName_def >> xmlFileName_def;

  if (ft2FileName == "") ft2FileName = ft2FileName_def;
  if (xmlFileName == "") xmlFileName = xmlFileName_def;
  if (truncation == 0) truncation = truncation_def;

  ft2Verify v;
  v.analyzeFt2(ft2FileName.c_str());
  v.writeXmlFile(xmlFileName.c_str(),truncation);
}

