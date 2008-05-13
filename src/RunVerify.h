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
#include <xercesc/util/XercesDefs.hpp>
#include "xercesc/dom/DOMElement.hpp"

/**
 * \class RunVerify
 * \brief RunVerify verification of digi data
 * \author M.E.
 */

using std::list;
using std::vector;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class DomElement;

class EvtError{
 friend class RunVerify;
 public:
  EvtError(int evtNumber, string errName, int errValue, int epuNumber);
  EvtError(int evtNumber, string errName, int errValue);
  ~EvtError();

 private:
  int m_evtNumber;
  std::string m_errName;
  int m_errValue;
  int m_epuNumber;
};

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

  int m_nEvent;
  vector<EpuDatagrams> m_epuList;
  std::list<std::string> m_errorTypes;
  std::list<EvtError> m_evtErrors;
  // evts/datagram histogram for each EPU
  TH1F* m_datagrams[MaxEpuNumber];
};

#endif
