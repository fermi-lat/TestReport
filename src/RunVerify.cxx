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
    m_nEvent(0),
    m_latcKey(0),
    m_firstGemSequence(0),
    m_thisGemSequence(0),
    m_firstGemElapsed(0),
    m_thisGemElapsed(0),
    m_firstGemLivetime(0),
    m_thisGemLivetime(0),
    m_thisGemPrescaled(0),
    m_thisGemDiscarded(0),
    m_thisGemDeadzone(0),
    m_thisGpsCurrent(0),
    m_thisGpsPrevious(0),
    m_thisTimeStamp(0)
{ 
  // initialize ROOT
  if(gROOT == 0) {
    static TROOT g_root("report", "report");
  }
  //gROOT->SetBatch();  // initialize ROOT

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
    m_datagrams[i] = new TH1F(ptr.c_str(),title.c_str(),100,0,500);
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

EvtError::EvtError(string errName, int errValue, int epuNumber, int datagramNbr)
  : m_errName(errName),
    m_errValue(errValue),
    m_epuNumber(epuNumber),
    m_datagramNbr(datagramNbr)
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

int RunVerify::analyzeDigi(const char* digiFileName="digi.root", bool completeRun=false)
{
  int nDigi  = -1;
  int nbrEventsDG[MaxEpuNumber];
  int atLeastOneEvt[MaxEpuNumber];
  bool runHasGaps = false;
  bool wrongBounds = false;
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
    m_digiTree->SetBranchStatus("*",0);
    m_digiTree->SetBranchStatus("m_ccsds",1);
    m_digiTree->SetBranchStatus("m_eventId",1);
    m_digiTree->SetBranchStatus("m_runId",1);
    m_digiTree->SetBranchStatus("m_metaEvent",1);
    m_digiTree->SetBranchStatus("m_timeStamp",1);
    m_digiTree->SetBranchStatus("m_gem",1);
    m_digiTree->SetBranchStatus("m_summary",1);
  } else {
    cout << "ERROR: no digi file " << digiFileName << "opened!" << endl;
    return 1;
  }
  
  // Make sure we have events in the digi file:
  nDigi = (int) m_digiTree->GetEntries();
  cout << "Number of events in " << digiFileName << " : " << nDigi << endl;
  m_nEvent = nDigi;
  if(nDigi < 1){
    cout << "ERROR: no events in digi file " << digiFileName << endl;
    return 2;
  }
 
  for (int iLoop = 0; iLoop < MaxEpuNumber; ++iLoop) {
    idDatagram[iLoop]    = 0;
    nbrEventsDG[iLoop]   = 0;
    atLeastOneEvt[iLoop] = 0;
  }

  // Loop over events:
  for(int iEvent = 0; iEvent != m_nEvent; ++iEvent) {

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
    int evtGemId = (int) m_digiEvent->getEventId();

    // check the LatcKey
    if (m_digiEvent->getMetaEvent().keys() != 0) { 
      unsigned int tmpLatcKey = m_digiEvent->getMetaEvent().keys()->LATC_master(); 
      if (m_latcKey == 0){
        m_latcKey = tmpLatcKey;
	cout << "LatcKey found: " << m_latcKey << endl;
      } else if (tmpLatcKey != m_latcKey){
	//cout << "ERROR! LatcKey changed from: " << m_latcKey  <<" to: " << tmpLatcKey << endl;
        m_latcKey = tmpLatcKey;
	errorName = "LATC_KEY_CHANGE"; // ['The LATC Master key has changed during the run'] 
        EvtError* evt_e = new EvtError(errorName,m_latcKey,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
    } 

    // check the compression level (should always be 8)
    // FSW B2-1-0 was activated on 2009-09-17 at 13:25:19 (274886721)
    if ( m_digiEvent->getRunId() > 274886721 && m_digiEvent->getMetaEvent().compressionLevel() != 8 ){
      //cout << "Compression level = "<< m_digiEvent->getMetaEvent().compressionLevel() <<" for event: " << iEvent << endl;
      errorName = "COMPRESSION_LEVEL"; // ['The FSW Compression Level is different from 8'] 
      EvtError* evt_e = new EvtError(errorName,m_digiEvent->getMetaEvent().compressionLevel(),-1,-1);
      m_evtMap[evtGemId].push_back(evt_e);
      m_errMap[errorName].push_back(iEvent);
    }

    // we shouldn't have TEM bug events for periodic triggers 
    if ( m_digiEvent->getGem().getConditionSummary()&32 && m_digiEvent->getEventSummaryData().temBug() ){ 
      errorName = "TEM_BUG_PERIODIC"; // ['The TEM bug was present for a periodic event'] 
      EvtError* evt_e = new EvtError(errorName,m_digiEvent->getEventSummaryData().temBug(),-1,-1);
      m_evtMap[evtGemId].push_back(evt_e);
      m_errMap[errorName].push_back(iEvent);
    }

    // check the CCSDS timestamp 
    double ccsdsTime = m_digiEvent->getCcsds().getUtc();
    // Time from Mission elapsed time to Unix time:
    double deltaTimeUgly = 978307200;
    double timeStamp = m_digiEvent->getTimeStamp();
    // Leap second from Dec 31st, 2005
    if (timeStamp > 157766400) deltaTimeUgly--; 
    // Leap second from Dec 31st, 2008
    if (timeStamp > 252460800) deltaTimeUgly--;
    // Leap second from Jun 30th, 2012
    if (timeStamp > 362793601) deltaTimeUgly--
    timeStamp = timeStamp + deltaTimeUgly;

    if ( ccsdsTime < timeStamp ){
      //cout << "ERROR! CCSDS packet time: " << (int)ccsdsTime  <<" smaller than timestamp: " << (int)timeStamp << endl;
      errorName = "CCSDS_EARLY_TIMESTAMP"; // ['The CCSDS Time is earlier than the event TimeStamp']
      EvtError* evt_e = new EvtError(errorName,int(ccsdsTime-timeStamp),cpuNbr,-1);
      m_evtMap[evtGemId].push_back(evt_e);
      m_errMap[errorName].push_back(iEvent);
    }

    // check the Gem/GPS Scalers and Counters
    unsigned long long tmpGemSequence = m_digiEvent->getMetaEvent().scalers().sequence();
    unsigned long long tmpGemElapsed = m_digiEvent->getMetaEvent().scalers().elapsed();
    unsigned long long tmpGemLivetime = m_digiEvent->getMetaEvent().scalers().livetime();
    unsigned long long tmpGemPrescaled = m_digiEvent->getMetaEvent().scalers().prescaled();
    unsigned long long tmpGemDiscarded = m_digiEvent->getMetaEvent().scalers().discarded();
    unsigned long long tmpGemDeadzone = m_digiEvent->getMetaEvent().scalers().deadzone();
    unsigned int tmpGpsCurrent = m_digiEvent->getMetaEvent().time().current().timeSecs();
    unsigned int tmpGpsPrevious = m_digiEvent->getMetaEvent().time().previous().timeSecs();
    double tmpTimeStamp = m_digiEvent->getTimeStamp();

    if (iEvent == 0){
      m_firstGemSequence = tmpGemSequence;
      m_firstGemElapsed = tmpGemElapsed;
      m_firstGemLivetime = tmpGemLivetime;
    } else {
      if ( iEvent > 1 && iEvent < m_nEvent-1 && tmpGemSequence <= m_thisGemSequence){
	  errorName = "GEM_SEQUENCE_BACKWARDS"; // ['The GEM Sequence counter is going backwards'] 
          EvtError* evt_e = new EvtError(errorName,m_thisGemSequence-tmpGemSequence,-1,-1);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGemElapsed < m_thisGemElapsed){
	errorName = "GEM_ELAPSED_BACKWARDS"; // ['The GEM Elapsed Time counter is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemElapsed-tmpGemElapsed,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (iEvent > 1 && iEvent < m_nEvent-1 && (tmpGemElapsed >= m_thisGemElapsed) && (tmpGemElapsed - m_thisGemElapsed) < 530){
	errorName = "GEM_ELAPSED_UNPHYSICAL"; // ['The Elapsed Time from the previous event is less than 530 ticks'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemElapsed-tmpGemElapsed,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGemLivetime < m_thisGemLivetime){
	errorName = "GEM_LIVETIME_BACKWARDS"; // ['The GEM Livetime counter is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemLivetime-tmpGemLivetime,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGemPrescaled < m_thisGemPrescaled){
	errorName = "GEM_PRESCALED_BACKWARDS"; // ['The GEM Prescaled counter is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemPrescaled-tmpGemPrescaled,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGemDiscarded < m_thisGemDiscarded){
	errorName = "GEM_DISCARDED_BACKWARDS"; // ['The GEM Discarded counter is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemDiscarded-tmpGemDiscarded,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGemDeadzone < m_thisGemDeadzone){
	errorName = "GEM_DEADZONE_BACKWARDS"; // ['The GEM Deadzone counter is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGemDeadzone-tmpGemDeadzone,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGpsCurrent < m_thisGpsCurrent){
	errorName = "GPS_CURRENT_BACKWARDS"; // ['The GPS time of the Current event is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGpsCurrent-tmpGpsCurrent,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpGpsPrevious < m_thisGpsPrevious){
	errorName = "GPS_PREVIOUS_BACKWARDS"; // ['The GPS time of the Previous event is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,m_thisGpsPrevious-tmpGpsPrevious,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if (tmpTimeStamp < m_thisTimeStamp){
	errorName = "EVT_TIMESTAMP_BACKWARDS"; // ['The Event TimeStamp (MET) is going backwards'] 
        EvtError* evt_e = new EvtError(errorName,int(m_thisTimeStamp-tmpTimeStamp),-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      }
      if ( (tmpGemElapsed-m_thisGemElapsed) > 0 ) {
        double tmpLiveRatio = (float) (tmpGemLivetime-m_thisGemLivetime)/(tmpGemElapsed-m_thisGemElapsed);
        if ( tmpLiveRatio > 1 ){
          errorName = "EVT_LIVETIME_RATIO"; // ['The Livetime since the previous event is greater than the Elapsed Time'] 
          EvtError* evt_e = new EvtError(errorName,int(tmpLiveRatio),-1,-1);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(iEvent);
        }
      }
    }
    m_thisGemSequence = tmpGemSequence;
    m_thisGemElapsed = tmpGemElapsed;
    m_thisGemLivetime = tmpGemLivetime;
    m_thisGemPrescaled = tmpGemPrescaled;
    m_thisGemDiscarded = tmpGemDiscarded;
    m_thisGemDeadzone = tmpGemDeadzone;
    m_thisGpsCurrent = tmpGpsCurrent;
    m_thisGpsPrevious = tmpGpsPrevious;
    m_thisTimeStamp = tmpTimeStamp;

    // check total delta Id/delta time
    if (iEvent == m_nEvent-1){
      unsigned long long deltaGemSequence = tmpGemSequence + 1 - m_firstGemSequence;
      unsigned long long deltaGemElapsed = tmpGemElapsed - m_firstGemElapsed;
      unsigned long long deltaGemLivetime = tmpGemLivetime - m_firstGemLivetime;
      cout << "Gem Scalers -> Total Events Counted: " << deltaGemSequence << endl;
      cout << "Gem Scalers -> Total Elapsed Time (ticks): " << deltaGemElapsed << endl; 
      cout << "Gem Scalers -> Total Live Time (ticks): " << deltaGemLivetime << endl; 
      if ( m_nEvent > deltaGemSequence+2 ){
        cout << "ERROR! Number of events in Gem Scalers: " << deltaGemSequence << "; number of events in Digi File: " << m_nEvent << endl;
	errorName = "GEM_SEQUENCE_NEVENTS"; // ['The Digi file has more events than what counted by the GEM']  
        EvtError* evt_e = new EvtError(errorName,deltaGemSequence,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      } 
      if ( deltaGemElapsed > 0 && deltaGemLivetime > deltaGemElapsed ){
        cout << "ERROR! Livetime in Gem Scalers: " << deltaGemLivetime << " greater than elapsed time: " << deltaGemElapsed << endl;
	errorName = "GEM_LIVETIME_RATIO"; // ['The Livetime is greater than the Elapsed Time'] 
        EvtError* evt_e = new EvtError(errorName,deltaGemLivetime/deltaGemElapsed,-1,-1);
        m_evtMap[evtGemId].push_back(evt_e);
        m_errMap[errorName].push_back(iEvent);
      } 
    }

    // Gaps in datagram sequence number?
    if ((DatagramSeqNbr != m_epuList.at(cpuNbr).m_previousDatagram) && 
    		((DatagramSeqNbr-m_epuList.at(cpuNbr).m_previousDatagram)!=1) && atLeastOneEvt[cpuNbr]>0 ) {
      m_epuList.at(cpuNbr).m_datagramGaps++;
      errorName = "DATAGRAM_GAP"; // ['Gaps were found in the datagram sequence number'] 
      EvtError* evt_e = new EvtError(errorName,DatagramSeqNbr-m_epuList.at(cpuNbr).m_previousDatagram-1,cpuNbr,DatagramSeqNbr);
      m_evtMap[evtGemId].push_back(evt_e);
      m_errMap[errorName].push_back(iEvent);
      cout << "Warning! there was a gap in the datagram sequence number for " << m_epuList.at(cpuNbr).m_epuName << "! event " 
      	   << iEvent << ", datagram gap: " << DatagramSeqNbr << " - " << m_epuList.at(cpuNbr).m_previousDatagram << endl;  
      runHasGaps = true;
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
    // First event in first datagram? (NB: I generate the error only if the run is complete)!
    if (atLeastOneEvt[cpuNbr] == 0) {
      m_epuList.at(cpuNbr).m_listDatagrams.push_back(DatagramSeqNbr);
      m_epuList.at(cpuNbr).m_firstDatagram = DatagramSeqNbr;
      int firstDatagramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
      if (firstDatagramOpen == enums::Lsf::Open::Start) {
	m_epuList.at(cpuNbr).m_firstOpenAction = 1;
      } else {
        wrongBounds = true;
        if (completeRun) {
          errorName = "FIRST_DATAGRAM_OPENING"; // ['The first datagram was not opened for the appropriate reason'] 
          EvtError* evt_e = new EvtError(errorName,firstDatagramOpen,cpuNbr,DatagramSeqNbr);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(iEvent);
	}
        cout << "Warning! The fist datagram for " << m_epuList.at(cpuNbr).m_epuName 
	     << " was not opened because we started the run! The datagram opening action was " << firstDatagramOpen << endl;
      }
      if (m_epuList.at(cpuNbr).m_firstDatagram >0){
        wrongBounds = true;
        if (completeRun) {
          errorName = "FIRST_DATAGRAM_ID"; // ['The first datagram has not datagramID = 0'] 
          EvtError* evt_e = new EvtError(errorName,m_epuList.at(cpuNbr).m_firstDatagram,cpuNbr,DatagramSeqNbr);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(iEvent);
	}
        cout << "Warning! The fist datagram for " << m_epuList.at(cpuNbr).m_epuName 
	     << " did not have datagram ID = 0! The first datagram ID was " << m_epuList.at(cpuNbr).m_firstDatagram << endl;
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
	    errorName = "DROPPED_DATAGRAMS"; // ['Some datagrams were dropped (lost)'] 
            EvtError* evt_e = new EvtError(errorName,diff-1,iLoop,(*p));
            m_evtMap[-1].push_back(evt_e);
            m_errMap[errorName].push_back(-1);
            m_epuList.at(iLoop).m_counterMissingDatagrams = m_epuList.at(iLoop).m_counterMissingDatagrams + diff - 1;
     	    cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for " 
		 << m_epuList.at(iLoop).m_epuName << "!" << endl;
            runHasGaps = true;
	  }
	}
      }
    }
    if (m_epuList.at(iLoop).m_lastDatagramEvent != -1) {
      if (m_digiEvent) m_digiEvent->Clear();
      m_digiBranch->GetEntry(m_epuList.at(iLoop).m_lastDatagramEvent);
      int evtGemId = (int) m_digiEvent->getEventId();
      unsigned int DatagramSeqNbr = m_digiEvent->getMetaEvent().datagram().datagrams();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      if (lastActionDataGram == enums::Lsf::Close::Stop || lastActionDataGram == enums::Lsf::Close::Abort) {
        m_epuList.at(iLoop).m_lastCloseAction = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        wrongBounds = true;
	m_epuList.at(iLoop).m_lastDatagramFull = 1;
        if (completeRun) {
	  errorName = "LAST_DATAGRAM_FULL"; // ['The last datagram was full: data might be missing'] 
          EvtError* evt_e = new EvtError(errorName,lastReasonDataGram,iLoop,DatagramSeqNbr);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(m_epuList.at(iLoop).m_lastDatagramEvent);
        }
	cout << "Warning! The closing reason for the last datagram for " << m_epuList.at(iLoop).m_epuName 
	     << " was: Datagram Full -- Data might be lost!" << endl;
      }
      if (m_epuList.at(iLoop).m_lastCloseAction == 0) {
        wrongBounds = true;
        if (completeRun) {
          errorName = "LAST_DATAGRAM_CLOSING"; // ['The last datagram was not closed for the appropriate reason']
          EvtError* evt_e = new EvtError(errorName,lastActionDataGram,iLoop,DatagramSeqNbr);
          m_evtMap[evtGemId].push_back(evt_e);
          m_errMap[errorName].push_back(m_epuList.at(iLoop).m_lastDatagramEvent);
	}
        cout << "Warning! The last datagram for " << m_epuList.at(iLoop).m_epuName 
	     << " was not closed because we reached the end of run! The datagram closing action was " << lastActionDataGram << endl;
      }
      m_digiEvent->Clear();
    }
  }
  // Closing time:  
  if(m_digiFile) m_digiFile->Close();
  if (runHasGaps) return 3;
  if (wrongBounds) return 4;
  return 0;
}

Bool_t RunVerify::writeXmlFile(const char* fileName, bool completeRun, int truncated) const {

  DomElement elem = AcdXmlUtil::makeDocument("errorContribution");
  writeXmlHeader(elem, completeRun);
  writeXmlErrorSummary(elem, truncated);
  writeXmlEventSummary(elem, truncated);
  writeXmlFooter(elem);
  return AcdXmlUtil::writeIt(elem,fileName);

}

void RunVerify::writeXmlHeader(DomElement& node, bool completeRun) const {
  DomElement runStatus = AcdXmlUtil::makeChildNode(node,"Run");
  string isComplete("inProgress");
  if (completeRun) isComplete = "Complete";
  AcdXmlUtil::addAttribute(runStatus,"Status",isComplete.c_str());
  return;
}

void RunVerify::writeXmlFooter(DomElement& /* node */) const {
  // do nothing, for now
  return;
}

void RunVerify::writeXmlErrorSummary(DomElement& node, int truncation) const {
  DomElement errSummary = AcdXmlUtil::makeChildNode(node,"errorSummary");
  // loop on the errors to make the error summary list
  for (map< string, list<int> >::const_iterator it = m_errMap.begin(); it != m_errMap.end(); it++){
    DomElement errType = AcdXmlUtil::makeChildNode(errSummary,"errorType");
    AcdXmlUtil::addAttribute(errType,"code",(*it).first.c_str());
    AcdXmlUtil::addAttribute(errType,"quantity",(int)(*it).second.size());
    if ((int)(*it).second.size()>truncation) AcdXmlUtil::addAttribute(errType,"truncated","True");
    else AcdXmlUtil::addAttribute(errType,"truncated","False");
  }
}

void RunVerify::writeXmlEventSummary(DomElement& node, int truncation) const {
  DomElement evtSummary = AcdXmlUtil::makeChildNode(node,"eventSummary");
  AcdXmlUtil::addAttribute(evtSummary,"num_processed_events",m_nEvent);
  AcdXmlUtil::addAttribute(evtSummary,"num_error_events",(int)m_evtMap.size());
  map< string, int > m_errCounter;
  for (map< string, list<int> >::const_iterator it = m_errMap.begin(); it != m_errMap.end(); it++){
    m_errCounter[(*it).first.c_str()] = 0;
    if ((int)(*it).second.size()>truncation) AcdXmlUtil::addAttribute(evtSummary,"truncated","True");
    else AcdXmlUtil::addAttribute(evtSummary,"truncated","False");
  }
  for (map< int, list<EvtError*> >::const_iterator it = m_evtMap.begin(); it != m_evtMap.end(); it++){  
    // check if there is at least one error that didn't reach truncation limit for this event
    bool write_event = false;
    for (list<EvtError*>::const_iterator ie = (*it).second.begin(); ie != (*it).second.end(); ie ++){
      if (m_errCounter[(*ie)->m_errName.c_str()] <= truncation) write_event = true;
    }
    if (write_event) {
      DomElement evtError = AcdXmlUtil::makeChildNode(evtSummary,"errorEvent");
      if ( (*it).first > -1 ) AcdXmlUtil::addAttribute(evtError,"evtGemId",(int)(*it).first);
      else AcdXmlUtil::addAttribute(evtError,"evtGemId","noEvt");
      for (list<EvtError*>::const_iterator ie = (*it).second.begin(); ie != (*it).second.end(); ie ++){
        DomElement errDetail = AcdXmlUtil::makeChildNode(evtError,"error");
        AcdXmlUtil::addAttribute(errDetail,"code",(*ie)->m_errName.c_str());
        AcdXmlUtil::addAttribute(errDetail,"value",(*ie)->m_errValue);
        if ((*ie)->m_epuNumber > -1) AcdXmlUtil::addAttribute(errDetail,"cpu",m_epuList.at((*ie)->m_epuNumber).m_epuName.c_str());
        if ((*ie)->m_datagramNbr > -1) AcdXmlUtil::addAttribute(errDetail,"datagram",(*ie)->m_datagramNbr);
        m_errCounter[(*ie)->m_errName.c_str()]++;
      }
    }
  }
}

