#include <string>
#include <fstream>
#include <iostream>
#include <getopt.h>
#include "meritVerify.h"

using std::cout;
using std::endl;

int main(int argn, char** argv)
{
  std::string meritFileName, xmlFileName, optionFileName;
  std::string meritFileName_def, xmlFileName_def;
  int opt, truncation=0, truncation_def=100;
 
  while ( (opt = getopt(argn, argv, "o:f:x:t:")) != EOF ) {
    switch (opt) {
      case 'o':   // Option file name
        optionFileName = string(optarg);
        break;
      case 'f':   // merit file name      
        meritFileName = string(optarg);
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
    cout << "meritVerify only takes options, not bare arguments; using default settings." << endl;
  }

  if (optionFileName == "") optionFileName = "../src/meritVerifyOption.dat";
  std::ifstream optionFile(optionFileName.c_str());
  optionFile >> meritFileName_def >> xmlFileName_def;

  if (meritFileName == "") meritFileName = meritFileName_def;
  if (xmlFileName == "") xmlFileName = xmlFileName_def;
  if (truncation == 0) truncation = truncation_def;

  meritVerify v;
  v.analyzeMerit(meritFileName.c_str());
  v.writeXmlFile(xmlFileName.c_str(),truncation);
}

