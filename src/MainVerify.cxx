#include <string>
#include <fstream>
#include <getopt.h>
#include "RunVerify.h"

int main(int argn, char** argv)
{
  std::string digiRootFile, xmlFileName, histoFileName, version, grVersion, optionFileName;
  std::string digiRootFile_def, xmlFileName_def, histoFileName_def, version_def, grVersion_def;
  int opt;
 
  while ( (opt = getopt(argn, argv, "o:d:x:r:v:g:")) != EOF ) {
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
      case 'v':   // program version     
        version = string(optarg);
        break;
      case 'g':   // GlastRelease version     
        grVersion = string(optarg);
        break;
    } 
  }

  if ( argn - optind > 0 ) {
    cout << "RunVerify only takes options, not bare arguments; using default settings." << endl;
  }

  if (optionFileName == "") optionFileName = "../src/VerifyOption.dat";
  std::ifstream optionFile(optionFileName.c_str());
  optionFile >> digiRootFile_def >> xmlFileName_def >> histoFileName_def >> version_def >> grVersion_def;

  if (digiRootFile == "") digiRootFile = digiRootFile_def;
  if (xmlFileName == "") xmlFileName = xmlFileName_def;
  if (histoFileName == "") histoFileName = histoFileName_def;
  if (version == "") version = version_def;
  if (grVersion == "") grVersion = grVersion_def;

  RunVerify v(xmlFileName.c_str(), histoFileName.c_str(), version.c_str(), grVersion.c_str());
  v.analyzeDigi(digiRootFile.c_str());
  v.generateXml();
}

