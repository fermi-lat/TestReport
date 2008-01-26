#ifndef RunVerify_Header
#define RunVerify_Header
#define MaxEpuNumber 5

#include <fstream>
#include <string>
#include <list>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "digiRootData/DigiEvent.h"

/**
 * \class RunVerify
 * \brief RunVerify verification of digi data
 * \author M.E.
 */

using std::list;
using std::vector;

class EpuDatagrams {
 friend class RunVerify;
 public:
  EpuDatagrams(const char* epuName);
  ~EpuDatagrams();

 private:
  std::string m_epuName;
  int m_counterMissingDatagrams;
  int m_nbrEventsDatagram;
  unsigned int m_firstDatagram;
  unsigned int m_previousDatagram;
  int m_firstOpenAction;
  int m_lastCloseAction;
  int m_lastDatagramFull;
  int m_datagramGaps;
  int m_lastDatagramEvent;
  std::list<int> m_listDatagrams;
};

typedef vector<EpuDatagrams> EpuList;

class RunVerify {
 //friend class EpuDatagrams;
 public:
  RunVerify(const char* dir, const char* prefix, const char* version, const char* grVersion);
  ~RunVerify();

  void analyzeDigi(const char* digiFileName);
  void generateXml();

 private:
  void writeHeader();
  void writeTail();

  /// directory where output files are generated
  std::string m_dir;
  /// prefix to the output files
  std::string m_prefix;
  /// version of the RunVerify package
  std::string m_version;
  /// version of the GlastRelease package
  std::string m_grVersion;
 
  TFile* m_root;
  std::ofstream* m_xml;

  TFile* m_digiFile;
  TTree* m_digiTree;
  TBranch* m_digiBranch;
  DigiEvent* m_digiEvent;

  int m_nEvent;
  EpuList m_epuList;
  // evts/datagram histogram for each EPU
  TH1F* m_datagrams[MaxEpuNumber];
};


#endif
