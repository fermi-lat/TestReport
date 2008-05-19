#ifndef RunVerify_Header
#define RunVerify_Header
#define MaxEpuNumber 5

#include <fstream>
#include <string>
#include <list>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "digiRootData/DigiEvent.h"
#include <xercesc/util/XercesDefs.hpp>
#include "xercesc/dom/DOMElement.hpp"

/**
 * \class RunVerify
 * \brief RunVerify verification of digi data
 * \author M.E.
 */

using std::list;
using std::string;
using std::vector;
using std::map;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class DomElement;

class EvtError{
 friend class RunVerify;
 public:
  EvtError(string errName, int errValue, int epuNumber);
  ~EvtError();

 private:
  string m_errName;
  int m_errValue;
  int m_epuNumber;
};

class EpuDatagrams {
 friend class RunVerify;
 public:
  EpuDatagrams(const char* epuName);
  ~EpuDatagrams();

 private:
  string m_epuName;
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

class RunVerify {
 public:
  RunVerify(const char* histoFileName);
  ~RunVerify();

  void analyzeDigi(const char* digiFileName);
  
  // write errors to an xml file
  Bool_t writeXmlFile(const char* fileName) const;
  // write xml header
  void writeXmlHeader(DomElement& node) const;
  // write xml footer
  void writeXmlFooter(DomElement& node) const;
  // write error summary to xml file
  void writeXmlErrorSummary(DomElement& node) const;
  // write event summary to xml file
  void writeXmlEventSummary(DomElement& node) const;

 private:
  /// root output file
  std::string m_histoFileName;
 
  TFile* m_root;
  TFile* m_digiFile;
  TTree* m_digiTree;
  TBranch* m_digiBranch;
  DigiEvent* m_digiEvent;

  unsigned int m_latcKey; 
  unsigned int m_groundId; 
  int m_nEvent;
  vector<EpuDatagrams> m_epuList;
  map< string, list<int> > m_errMap;
  map< int, list<EvtError*> > m_evtMap;
  // evts/datagram histogram for each EPU
  TH1F* m_datagrams[MaxEpuNumber];
};

#endif
