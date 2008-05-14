#include <iostream>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <iterator>
#include <list>
#include <algorithm>

#include "TROOT.h"
#include "RunVerify.h"
#include "AcdXmlUtil.h"
#include "DomElement.h"
#include "TSystem.h"

#include "xmlBase/Dom.h"
#include "xmlBase/XmlParser.h"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMDocument.hpp" 
#include "xercesc/dom/DOMImplementation.hpp"


using std::cout;
using std::endl;
using std::string;
using std::list;
using std::vector;

RunVerify::RunVerify(const char* histoFileName)
  : m_histoFileName(histoFileName), 
    m_digiFile(0), 
    m_digiTree(0),
    m_digiBranch(0), 
    m_digiEvent(0), 
    m_nEvent(0)
{ 
  // initialize ROOT
  if(gROOT == 0) {
    static TROOT g_root("report", "report");
  }
  gROOT->SetBatch();  // initialize ROOT

  string r(m_histoFileName);
  m_root = new TFile(r.c_str(), "RECREATE");
  m_epuList.clear();
  m_errMap.clear();
  m_evtMap.clear();

  for (int i=0; i< MaxEpuNumber; i++){
    char e_name[5];
    if(i==enums::Lsf::Epu0) sprintf(e_name,"Epu0");
    if(i==enums::Lsf::Epu1) sprintf(e_name,"Epu1");
    if(i==enums::Lsf::Epu2) sprintf(e_name,"Epu2");
    if(i==enums::Lsf::Siu0) sprintf(e_name,"Siu0");
    if(i==enums::Lsf::Siu1) sprintf(e_name,"Siu1");
    EpuDatagrams* epu_d = new EpuDatagrams(e_name);
    m_epuList.push_back(*epu_d);
    string ptr("datagrams");
    ptr += m_epuList.at(i).m_epuName;
    string title("Number of events per datagram - ");
    title += m_epuList.at(i).m_epuName;
    m_datagrams[i] = new TH1F(ptr.c_str(),title.c_str(),101,-2,200);
  }
}

RunVerify::~RunVerify()
{
  if(m_root) {
    m_root->cd();
    m_root->Write(0, TObject::kOverwrite);
    m_root->Close();

    // when the file is deleted, all trees and histograms associated with it
    // also get deleted. So they should not be manually deleted again
    delete m_root;
  }
  delete m_digiFile;
  m_epuList.clear();
  m_errMap.clear();
  m_evtMap.clear();
}

EvtError::EvtError(string errName, int errValue, int epuNumber)
  : m_errName(errName),
    m_errValue(errValue),
    m_epuNumber(epuNumber)
{
}

EvtError::~EvtError() {
}

EpuDatagrams::EpuDatagrams(const char* epuName)
  : m_epuName(epuName),
    m_counterMissingDatagrams(0),
    m_firstDatagram(0),
    m_firstOpenAction(0),
    m_lastCloseAction(0),
    m_lastDatagramFull(0),
    m_datagramGaps(0)
{
  m_previousDatagram         = 0;
  m_nbrEventsDatagram        = 0;
  m_lastDatagramEvent        =-1;
}

EpuDatagrams::~EpuDatagrams() {
  m_listDatagrams.clear();
}

void RunVerify::analyzeDigi(const char* digiFileName="digi.root")
{
  int nDigi  = -1;
  int nbrEventsDG[MaxEpuNumber];
  int atLeastOneEvt[MaxEpuNumber];
  unsigned int idDatagram[MaxEpuNumber];
  std::string errorName;

  //open digi file
  if(digiFileName) m_digiFile = new TFile(digiFileName, "READ");
  if(m_digiFile && m_digiFile->IsZombie()) m_digiFile = 0;
  if(m_digiFile) {
    m_digiTree = (TTree*) m_digiFile->Get("Digi");
    m_digiEvent = 0;
    m_digiBranch = m_digiTree->GetBranch("DigiEvent");
    m_digiBranch->SetAddress(&m_digiEvent);
  } else {
    cout << "ERROR: no digi file " << digiFileName << "opened!" << endl;
    return;
  }
  
  // Make sure we have events in the digi file:
  nDigi = (int) m_digiTree->GetEntries();
  cout << "Number of events in " << digiFileName << " : " << nDigi << endl;
  m_nEvent = nDigi;
  if(nDigi < 1){
    cout << "ERROR: no events in digi file " << digiFileName << endl;
    return;
  }
 
  for (int iLoop = 0; iLoop < MaxEpuNumber; ++iLoop) {
    idDatagram[iLoop]    = 0;
    nbrEventsDG[iLoop]   = 0;
    atLeastOneEvt[iLoop] = 0;
  }

  // Loop over events:
  //for(int iEvent = 0; iEvent != m_nEvent; ++iEvent) {
  for(int iEvent = 0; iEvent != 20000; ++iEvent) {

    if (iEvent % 10000 == 0) {
      cout << "Event number " << iEvent << endl;
    }
    // Cleanup:
    if (m_digiEvent)  m_digiEvent->Clear();
    // Analyze digi file:
    m_digiBranch->GetEntry(iEvent);
    int cpuNbr = m_digiEvent->getMetaEvent().datagram().crate();
    unsigned int DatagramSeqNbr = m_digiEvent->getMetaEvent().datagram().datagrams();
    m_epuList.at(cpuNbr).m_nbrEventsDatagram++;
    m_epuList.at(cpuNbr).m_lastDatagramEvent = iEvent;

    // Gaps in datagram sequence number?
    if ((DatagramSeqNbr != m_epuList.at(cpuNbr).m_previousDatagram) && 
    		((DatagramSeqNbr-m_epuList.at(cpuNbr).m_previousDatagram)!=1)) {
      m_epuList.at(cpuNbr).m_datagramGaps++;
      errorName = "DATAGRAM_GAP"; 
      EvtError* evt_e = new EvtError(errorName,-1,cpuNbr);
      m_evtMap[iEvent].push_back(evt_e);
      m_errMap[errorName].push_back(iEvent);
      cout << "Warning! there was a gap in the datagram sequence number for " << m_epuList.at(cpuNbr).m_epuName << "! event " 
      	   << iEvent << ", datagram gap: " << DatagramSeqNbr << " - " << m_epuList.at(cpuNbr).m_previousDatagram << endl;  
    }

    // Fill the events per datagram histos      
    if (m_digiEvent->getMetaEvent().datagram().datagrams() == idDatagram[cpuNbr]) {
      nbrEventsDG[cpuNbr]++;
    } else { 
      m_datagrams[cpuNbr]->Fill(nbrEventsDG[cpuNbr]);
      idDatagram[cpuNbr] = m_digiEvent->getMetaEvent().datagram().datagrams();
      nbrEventsDG[cpuNbr] = 0;
    }
    // Last event:
    if (iEvent == (m_nEvent-1)) {
      for (int iLoop = 0; iLoop < MaxEpuNumber; iLoop++) {
        if (nbrEventsDG[iLoop] != 0) m_datagrams[iLoop]->Fill(nbrEventsDG[iLoop]);
      }
    }

    // Here I make a list of the datagrams and keep the opening reason for the first datagram:
    // First event in first datagram?
    if (atLeastOneEvt[cpuNbr] == 0) {
      m_epuList.at(cpuNbr).m_listDatagrams.push_back(DatagramSeqNbr);
      m_epuList.at(cpuNbr).m_firstDatagram = DatagramSeqNbr;
      int firstDatagramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
      if (firstDatagramOpen == enums::Lsf::Open::Start) {
	m_epuList.at(cpuNbr).m_firstOpenAction = 1;
      } else {
        errorName = "FIRST_DATAGRAM_OPENING";
        EvtError* evt_e = new EvtError(errorName,firstDatagramOpen,cpuNbr);
        m_evtMap[iEvent].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
        cout << "Warning! The fist datagram for " << m_epuList.at(cpuNbr).m_epuName 
	     << " was not opened because we started the run! The datagram opening reason was " << firstDatagramOpen << endl;
      }
      atLeastOneEvt[cpuNbr] = 1;
    }
    if (atLeastOneEvt[cpuNbr] == 1) {
       // push back to DataGram list, if not already there! 
       if (DatagramSeqNbr != m_epuList.at(cpuNbr).m_previousDatagram) {
         m_epuList.at(cpuNbr).m_listDatagrams.push_back(DatagramSeqNbr);
       }
    }

    // Keep datagram sequence numbers for previous event
    m_epuList.at(cpuNbr).m_previousDatagram = DatagramSeqNbr;
    // end of loop over events    
    m_digiEvent->Clear();
  }

  // Final loop over crates
  for (int iLoop = 0; iLoop < MaxEpuNumber; iLoop ++) {
    if (atLeastOneEvt[iLoop] == 1) {
      // Get rid of duplicates:
      m_epuList.at(iLoop).m_listDatagrams.sort();
      m_epuList.at(iLoop).m_listDatagrams.unique();
      // Check if Datagrams were actually missing or just out of order (we see gaps in both cases)
      std::list<int>::iterator p;
      for (p = m_epuList.at(iLoop).m_listDatagrams.begin(); p != m_epuList.at(iLoop).m_listDatagrams.end(); p++) {       
        if (p != m_epuList.at(iLoop).m_listDatagrams.begin()) {
          int diff = *p - *(--p);
          p++;
          if (diff > 1) {
	    errorName = "DROPPED_DATAGRAMS";
            EvtError* evt_e = new EvtError(errorName,diff-1,iLoop);
            m_evtMap[-1].push_back(evt_e);
            m_errMap[errorName].push_back(-1);
            m_epuList.at(iLoop).m_counterMissingDatagrams = m_epuList.at(iLoop).m_counterMissingDatagrams + diff - 1;
     	    cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for " 
		 << m_epuList.at(iLoop).m_epuName << "!" << endl;
	  }
	}
      }
    }
    if (m_epuList.at(iLoop).m_lastDatagramEvent != -1) {
      m_digiBranch->GetEntry(m_epuList.at(iLoop).m_lastDatagramEvent);
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      if (lastActionDataGram == enums::Lsf::Close::Stop) {
        m_epuList.at(iLoop).m_lastCloseAction = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_epuList.at(iLoop).m_lastDatagramFull = 1;
	errorName = "LAST_DATAGRAM_FULL";
        EvtError* evt_e = new EvtError(errorName,-1,iLoop);
        m_evtMap[m_epuList.at(iLoop).m_lastDatagramEvent].push_back(evt_e);
        m_errMap[errorName].push_back(m_epuList.at(iLoop).m_lastDatagramEvent);
        cout << "The closing reason for the last datagram for " << m_epuList.at(iLoop).m_epuName << " was: Datagram Full" << endl;
      }
      if (m_epuList.at(iLoop).m_lastCloseAction == 0 && m_epuList.at(iLoop).m_lastDatagramFull == 0) {
        errorName = "LAST_DATAGRAM_CLOSING";
        EvtError* evt_e = new EvtError(errorName,lastActionDataGram,iLoop);
        m_evtMap[m_epuList.at(iLoop).m_lastDatagramEvent].push_back(evt_e);
        m_errMap[errorName].push_back(m_epuList.at(iLoop).m_lastDatagramEvent);
        cout << "Warning! The last datagram for " << m_epuList.at(iLoop).m_epuName 
	     << " was not closed because we reached the end of run or because it was full! The datagram closing reason was " 
	     << lastReasonDataGram << " and the datagram closing action was " << lastActionDataGram << endl;
      }
      m_digiEvent->Clear();
    }
  }
  // Closing time:  
  if(m_digiFile) m_digiFile->Close();
}

Bool_t RunVerify::writeXmlFile(const char* fileName) const {

  DomElement elem = AcdXmlUtil::makeDocument("errorContribution");
  writeXmlHeader(elem);
  writeXmlErrorSummary(elem);
  writeXmlEventSummary(elem);
  writeXmlFooter(elem);
  return AcdXmlUtil::writeIt(elem,fileName);

}

void RunVerify::writeXmlHeader(DomElement& /* node */) const {
  // do nothing, for now
  return;
}

void RunVerify::writeXmlFooter(DomElement& /* node */) const {
  // do nothing, for now
  return;
}

void RunVerify::writeXmlErrorSummary(DomElement& node) const {
  DomElement errSummary = AcdXmlUtil::makeChildNode(node,"errorSummary");
  // loop on the errors to make the error summary list
  for (map< string, list<int> >::const_iterator it = m_errMap.begin(); it != m_errMap.end(); it++){
    DomElement errType = AcdXmlUtil::makeChildNode(errSummary,"errorType");
    AcdXmlUtil::addAttribute(errType,"code",(*it).first.c_str());
    AcdXmlUtil::addAttribute(errType,"quantity",(int)(*it).second.size());
  }
}

void RunVerify::writeXmlEventSummary(DomElement& node) const {
  DomElement evtSummary = AcdXmlUtil::makeChildNode(node,"eventSummary");
  AcdXmlUtil::addAttribute(evtSummary,"num_processed_events",m_nEvent);
  AcdXmlUtil::addAttribute(evtSummary,"num_error_events",(int)m_evtMap.size());
  if ((int)m_evtMap.size()>500)
    AcdXmlUtil::addAttribute(evtSummary,"truncated","True");
  else 
    AcdXmlUtil::addAttribute(evtSummary,"truncated","False");
  int evtCounter = 0;  
  for (map< int, list<EvtError*> >::const_iterator it = m_evtMap.begin(); it != m_evtMap.end() && evtCounter < 500; it++){  
    DomElement evtError = AcdXmlUtil::makeChildNode(evtSummary,"errorEvent");
    if ( (*it).first > -1 )
      AcdXmlUtil::addAttribute(evtError,"eventNumber",(int)(*it).first);
    else
      AcdXmlUtil::addAttribute(evtError,"eventNumber","noEvt");
    for (list<EvtError*>::const_iterator ie = (*it).second.begin(); ie != (*it).second.end(); ie ++){
      DomElement errDetail = AcdXmlUtil::makeChildNode(evtError,"error");
      AcdXmlUtil::addAttribute(errDetail,"code",(*ie)->m_errName.c_str());
      if ((*ie)->m_errValue > -1)
        AcdXmlUtil::addAttribute(errDetail,"value",(*ie)->m_errValue);
      if ((*ie)->m_epuNumber > -1)
        AcdXmlUtil::addAttribute(errDetail,"cpu",m_epuList.at((*ie)->m_epuNumber).m_epuName.c_str());
    }
    evtCounter++;
  }
}

