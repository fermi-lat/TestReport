// $Id$
//
// stl stuff
#include <list>
#include <map>
#include <string>
#include <vector>

//
// io stuff
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <getopt.h>


//
// for the tokenizer
#include "facilities/Util.h"


//
// ROOT
#include "TH1.h"
#include "TList.h"
#include "TFile.h"
#include "TKey.h"


//
// usage message
int usage() {

  using std::cout;
  using std::cerr;
  using std::endl;
  
  cout << endl
       << "MergeHistFiles" << endl
       << " Correctly merge a bunch of histogram files." << endl
       << endl;
  
  cout << "Usage:" << endl
       << "\t" << "MergeHistFiles -I <inputFile> -o <outputFile> -c <configFile>" << endl 
       << endl
       << "or"
       << endl
       << "\t" << "MergeHistFiles -i <file1> -i <file2> ... -o <outputFile> -c <configFile>" << endl 
       << endl;

  cout << "\tinputFile is a list of root histogram files, one per line" << endl
       << "\t\t'#' at the beginning of the line is treated as a comment" << endl
       << endl;

  cout << "\tconfigFile is a list of histogram name and merging method codes." << endl
       << "\t\tOne pair per line separated by a space." << endl
       << "\t\tfor example 'AcdInEfficMap Average'" << endl
       << "\t\t'#' at the beginning of the line is treated as a comment" << endl
       << "\t\t'XX' is expands to mean all towers 00 to 15" << endl
       << endl << endl 
       << "\tLegal codes for merging methods are " << endl
       << "\t\t'Sum' -> Just add the histograms together" << endl
       << "\t\t'Average' -> Take the weighted mean" << endl
       << endl << endl;

  return -1;
}
  
//
// to save typing
typedef std::map<std::string,int> strToIntMap;

//
// load up the merge codes
void 
initMergeCodeMap(strToIntMap& theMap) {

  if ( theMap.size() != 0 ) return;
  // Just sum up all the bins
  theMap[std::string("Sum")] = 0;
  // Take the weighted average of each bin
  theMap[std::string("Average")] = 1;
}

//
// load up the map of how to fill the histograms
int fillHistMap(const char* fileName, strToIntMap& theMap, const strToIntMap& mergeCodes) { 

  // open file
  std::ifstream inputFile(fileName);
  if ( ! inputFile.good() ) {
    std::cerr << "Problems opening file " << fileName << std::endl;
    return -1;
  }

  // grab one line
  const int bufSize(200); char buffer[bufSize];
  inputFile.getline(buffer,bufSize);


  // Definition of Name loops (tower->16, callayer ->8...)
  // The histogram names in config file will be scanned for the flags 
  // specified here (i.e. TowerLoop) and N names (i.e. 16 for TowerLoop) 
  // will be created, each of them with a different suffix (vector component).
  // The entity relating flags with components is latloops, which 
  // is defined right here

  typedef std::map<std::string,std::vector<std::string> > LoopMap;
  LoopMap latloops;
  
  std::vector<unsigned int> vLoopDim;
  std::vector<std::string> vLoopFlags;
  vLoopFlags.push_back("TowerLoop");
  vLoopDim.push_back(16);
  vLoopFlags.push_back("CalLayerLoop");
  vLoopDim.push_back(8);
  vLoopFlags.push_back("CalColumnLoop");
  vLoopDim.push_back(12);
  vLoopFlags.push_back("AcdTileLoop");
  vLoopDim.push_back(128);

  // special tower loop (not for monitoring)
  vLoopFlags.push_back("XX");
  vLoopDim.push_back(16);
  
  // create vector of indices
  char charindex[5];
  for(unsigned int iloop = 0; iloop < vLoopFlags.size()-1; iloop++){
    
    std::vector<std::string> vl;
    
    for(unsigned int i = 0; i < vLoopDim[iloop];i++){
      sprintf(charindex,"%d",i);
      vl.push_back(charindex);
    }
    latloops[vLoopFlags[iloop]] = vl;
  }

  // The XX loop is a special one... 

  std::vector<std::string> vl;
  vl.push_back("00");
  vl.push_back("01");
  vl.push_back("02");
  vl.push_back("03");
  vl.push_back("04");
  vl.push_back("05");
  vl.push_back("06");
  vl.push_back("07");
  vl.push_back("08");
  vl.push_back("09");
  vl.push_back("10");
  vl.push_back("11");
  vl.push_back("12");
  vl.push_back("13");
  vl.push_back("14");
  vl.push_back("15");

  latloops[vLoopFlags[vLoopFlags.size()-1]] = vl;
  
  // print latloops
  /*
  for(LoopMap::const_iterator itr = latloops.begin();
      itr != latloops.end(); itr++){

    std::cout << "Printing components for loop " 
	      << itr->first.c_str() << std::endl << std::endl;
    
    for(unsigned int index = 0; index < itr->second.size();index++){
      std::cout << itr->second[index].c_str() << "\t";
    }
    std::cout << std::endl << std::endl;
    
  }
  */


  // loop until EOF
  while ( ! inputFile.eof() ) {

    // ignore comment lines (start with '#')
    if ( buffer[0] == '#' ) {
      inputFile.getline(buffer,bufSize);
      continue;
    }

    // tokenize the line, make sure that there are 2 tokens
    std::string inputLine(buffer);
    std::vector <std::string> tokens;
    facilities::Util::stringTokenize(inputLine, ": ", tokens);

    // ignore blank lines
    if ( tokens.size() == 0 ) {
      inputFile.getline(buffer,bufSize);
      continue;
    }
    if ( tokens.size() > 2 ) {
      std::cerr << "Can not parse input line " << inputLine << std::endl;
      return -2;
    }

    // make sure the line make sense, ie. that the merge code exists
    strToIntMap::const_iterator itrFind = mergeCodes.find( tokens[1] );
    if ( itrFind == mergeCodes.end() ) {
      std::cerr << "Could not find merge type code for " << inputLine << std::endl;
      return -4;
    }  

   
    // Checking (and making) loops using object latloops (defined above)
    /*
    two steps process
    1- Creation of object LoopMap LoopsInHistoName;
    2 - Replacing names with loops and filling theMap object
    */

    // Step 1
    // Creation and filling of object containing the loops to 
    // be done for this particular histogram name
    LoopMap LoopsInHistoName;
    std::string::size_type spos(tokens[0].size());
    for(LoopMap::const_iterator itr = latloops.begin();
	itr != latloops.end(); itr++){
      spos = tokens[0].find(itr->first.c_str());
      if(spos < tokens[0].size()){
	// this loop is present
	LoopsInHistoName[itr->first]=itr->second;
      }
      
    }
  
    // tmp
    /*
    std::cout << "Loops present for histo name " << tokens[0].c_str() << std::endl;
    for(LoopMap::const_iterator itr = LoopsInHistoName.begin();
	itr != LoopsInHistoName.end(); itr++){
      std::cout << itr->first.c_str() << std::endl;
    }
    */
    // endtmp
    
   
    // Step 2
    // Replacing names with loops and filling theMap object

    if(LoopsInHistoName.size() <1){
      // std::cout << "No loops" << std::endl;
      theMap[ tokens[0] ] = itrFind->second; // no loops to be done
    } 
    else{
      std::vector<std::string> vhistoname;
      std::vector<std::string> vhistonametmp;
      vhistoname.push_back(tokens[0]);
      
      for(LoopMap::const_iterator itr = LoopsInHistoName.begin();
	  itr != LoopsInHistoName.end(); itr++){
	spos = tokens[0].find(itr->first.c_str());
	
	if(spos > tokens[0].size()){
	  std::cout << "MergeHistFiles::fillHistMap(): ERROR" << std::endl
		    << "Loop with flag "<<  itr->first.c_str() << " is NOT present in " 
		    << "histo name " << tokens[0].c_str() << std::endl
		    << "This should not happen at this point... aborting."<< std::endl;
	  assert(0);
	}
      
	// drop components from vhistoname to vhistonametmp
	vhistonametmp.clear();
	for(unsigned int iname = 0; iname < vhistoname.size();iname++)
	  vhistonametmp.push_back(vhistoname[iname]);
	vhistoname.clear();
	
	for(unsigned int iname = 0; iname < vhistonametmp.size();iname++){
	  for(unsigned int index = 0; index < itr->second.size();index++){
	    std::string tmpstring = vhistonametmp[iname];
	    tmpstring.replace(spos,itr->first.size(),itr->second[index].c_str());
	    // std::cout << "String after replacement: " << tmpstring.c_str() << std::endl;
	    vhistoname.push_back(tmpstring);
	  }
	}
      }
      // tmp
      /*
      std::cout << "MergeHistFiles::fillHistMap(): info" << std::endl
		<< "Histogram names craeted after loop scan are the following ones: "
		<< std::endl
		<< std::endl;
      for(unsigned int i = 0; i < vhistoname.size();i++)
	std::cout << vhistoname[i].c_str() << std::endl;
      */
      // endtmp
      
      for(unsigned int i = 0; i < vhistoname.size();i++)
	theMap[vhistoname[i]] = itrFind->second;
      
    }
  
  
    // ok, on to the next line
    inputFile.getline(buffer,bufSize);
  }

  // clean up and return the number of histograms we expect to merge
  inputFile.close();
  return theMap.size();
}


// load up the list of input files from a text file
int 
getInputFilesFromTxtFile(const char* fileName, std::list<std::string>& fileNames){

  // open the file
  ifstream inputFile(fileName);
  if ( ! inputFile.good() ) {
    std::cerr << "Problems opening file " << fileName << std::endl;
    return -1;
  }
  
  // for the number of files 
  int n(0);

  // grab the first line
  const int bufSize(200); char buffer[bufSize];
  inputFile.getline(buffer,bufSize);

  // loop until EOF
  while ( ! inputFile.eof() ) {

    // ignore comment lines (which start with '#')
    if ( buffer[0] == '#' ) {
      inputFile.getline(buffer,bufSize);
      continue;
    }

    // tokenize and make sure it looks ok
    std::string inputLine(buffer);
    std::vector <std::string> tokens;
    facilities::Util::stringTokenize(inputLine, ": ", tokens);
    if ( tokens.size() == 0 ) {
      inputFile.getline(buffer,bufSize);
      continue;
    }
    if ( tokens.size() > 1 ) {
      std::cerr << "Can not parse input line " << inputLine << std::endl;
      return -2;
    }

    // looks ok, add to the list of filenames
    n++;
    fileNames.push_back(tokens[0]);
    inputFile.getline(buffer,bufSize);

  }

  // clean up and return the number of files added to list
  inputFile.close();
  return n;
}

//
// open the input files and store them on a TList
int getInputFiles(TList& l, const std::list<std::string>& fileNames) {

  int n(0);
  for ( std::list<std::string>::const_iterator itr = fileNames.begin();
	itr != fileNames.end(); itr++ ) {

    // make sure that file opens ok
    TFile* newFile = TFile::Open(itr->c_str());
    if ( newFile == 0 || newFile->IsZombie() ) {
      std::cerr << "Failed to open input file " << *itr << std::endl;
      return -1;
    }

    l.Add(newFile);
    n++;
  }

  // return the number of opened files
  return n;  
}

//
//
int mergeBySumming(TH1* hist, TList& sourcelist, TFile *first_source, const TString& path) {

  int nMerged(1);

  // loop over all source files and add the content of the
  // correspondant histogram to the one pointed to by "h1"
  TFile *nextsource = (TFile*)sourcelist.After( first_source );
  while ( nextsource ) {
        
    // make sure we are at the correct directory level by cd'ing to path
    nextsource->cd( path );
    TH1 *h2 = (TH1*)gDirectory->Get( hist->GetName() );
    if ( h2 ) {
      hist->Add( h2 );
      nMerged++;
      delete h2; // don't know if this is necessary, i.e. if 
      // h2 is created by the call to gDirectory above.
    }	
    nextsource = (TFile*)sourcelist.After( nextsource );
  }
  
  return nMerged;
}

//
//
int mergeByAveraging(TH1* hist, TList& sourcelist, TFile *first_source, const TString& path) {

  int nMerged = mergeBySumming(hist,sourcelist,first_source,path);
  
  Double_t scaleFactor = 1./ ((Double_t)nMerged);
  hist->Scale(scaleFactor);
  return nMerged;
}


//
// merge the histograms
//   mergeMap is the map<string,code> that tells us how to merge each 
//   histogram by name
int merge(TDirectory *target, TList& sourcelist, const strToIntMap& mergeMap) {

  // total number of histograms touched
  int nMergedTotal(0);

  // various ROOT stuff copied from hadd
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist.First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    // read object from first source file
    first_source->cd( path );
    TObject *obj = key->ReadObj();    
    
    // make sure the object is a histogram
    if ( obj->IsA()->InheritsFrom( "TH1" ) ) {

      TH1 *h1 = (TH1*)obj;

      // Make sure it is in the merge list
      std::string theName(key->GetName());
      strToIntMap::const_iterator itrFind = mergeMap.find(theName);
      if ( itrFind == mergeMap.end() ) {
	continue;      
      }
      int mergeType = itrFind->second;

           
      UInt_t nMerged(0);
      // merge according to type
      switch (mergeType) {
      case 0:
	nMerged = mergeBySumming(h1,sourcelist,first_source,path);
	break;
      case 1:
	nMerged = mergeByAveraging(h1,sourcelist,first_source,path);
	break;
      }

      nMergedTotal += nMerged;

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      std::cout << "Found subdirectory " << obj->GetName() << std::endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      merge( newdir, sourcelist, mergeMap);

    } else {

      // object is of no type that we know or can handle
      std::cout << "Unknown object type, name: " 
		<< obj->GetName() << " title: " << obj->GetTitle() << std::endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();      
      obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->Write();
  return nMergedTotal;
}


//
//
// OK, here is main.
int main(int argn, char** argc) {

  // stuff about which hists to merge and how to do so
  std::string configFile;
  strToIntMap mergeCodeMap;
  strToIntMap histMergeCodes;

  // stuff about input files
  std::list<std::string> inputFileList;
  std::string inputTextFile;
  std::string inputFile;
  TList inputList;
  int nInputFiles(0);

  // stuff about output files
  TFile* outputTFile(0);
  std::string outputFile;

  // parse options
  //char* endPtr;  
  int opt;
  while ( (opt = getopt(argn, argc, "ho:i:I:c:")) != EOF ) {
    switch (opt) {
    case 'h':   // help      
      usage();
      return 0;
    case 'o':   //  output
      outputFile = std::string(optarg);
      break;
    case 'I':   // input text file (with list of input root files)
      inputTextFile = std::string(optarg);
      break;
    case 'c':   // configuration file
      configFile = std::string(optarg);
      break; 
    case 'i':   // input root file (can be used many times)
      inputFile = std::string(optarg);
      inputFileList.push_back(inputFile);
      nInputFiles++;
      break;
    case '?':
      usage();
      return 2;
    default:
      std::cerr << opt << " not parsable..." << std::endl;
      std::cerr << "Try mergeHistFiles -h" << std::endl;
      return 2;
    }
  }

  // read the input files the text list of input files
  if ( inputTextFile.length() != 0 ) {
    int nRead = getInputFilesFromTxtFile(inputTextFile.c_str(),inputFileList);
    if ( nRead < 0 ) return 3;    
    nInputFiles += nRead;
  }

  // no inputs, fail
  if ( inputFileList.size() == 0 ) {
    std::cerr << "No input files specifiend. Use -I <textFile> or -i <filename>" << std::endl;
    return 4;
  }
  
  // open the input files, or die trying
  int nOpened = getInputFiles(inputList,inputFileList);
  if ( nOpened != nInputFiles ) {
    return 5;
  }

  // no output file, fail
  if ( outputFile == "" ) {
    std::cerr << "Output file not specified, use -o <filename>" << std::endl;
    return 6;
  }

  // open the output file, or die trying
  outputTFile = TFile::Open(outputFile.c_str(),"CREATE");
  if ( outputTFile == 0 || outputTFile->IsZombie() ) {
    std::cerr << "Problems opening output file " << outputFile << std::endl;
    return 7;
  }

  // no config file, fail
  if ( configFile == "" ) {
    std::cerr << "Config file not specified, use -c <filename>" << std::endl;
    return 8;
  }

  initMergeCodeMap(mergeCodeMap);  
  // open the config file, or die trying
  int mapSize = fillHistMap(configFile.c_str(),histMergeCodes,mergeCodeMap);
  if ( mapSize < 0 ) { 
    return 9;
  }

  // actually do the merging
  int nMerged = merge(outputTFile,inputList,histMergeCodes);
  std::cout << "Number of input file: " << nInputFiles << std::endl;
  std::cout << "Number of histograms: " << mapSize << std::endl;
  std::cout << "Number merged       : " << nMerged << std::endl;

  // all good, finish
  return 0;
}
