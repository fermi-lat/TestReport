#include <string>
#include <fstream>
#include "RunVerify.h"

int main(int argc, char** argv)
{
  std::string optionFileName;
  if(argc == 1) {
    optionFileName = "../src/VerifyOption.dat";
  }
  else {
    optionFileName = argv[1];
  }
  std::ifstream optionFile(optionFileName.c_str());

  std::string digiRootFile, outDir, prefix, version, grVersion;

  optionFile >> digiRootFile >> outDir >> prefix >> version >> grVersion;

  RunVerify v(outDir.c_str(), prefix.c_str(), version.c_str(), grVersion.c_str());
  v.analyzeDigi(digiRootFile.c_str());
  v.generateXml();
}

