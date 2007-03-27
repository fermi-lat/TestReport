#include <iostream>
#include <cstdio>
#include <strings.h>
#include <stdexcept>
#include "TROOT.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TestReport.h"
#include "ToString.h"
#include "TStyle.h"
#include "TLine.h"
#include "TMarker.h"
#include "TFrame.h"
#include "TLatex.h"
#include "Geo.h"
#include <list>
#include <iterator>

using std::cout;
using std::endl;
using std::string;

Float_t TestReport::efficDivide(TH1& top, const TH1& bot, Bool_t inEffic) {

  Int_t nt = top.GetNbinsX();
  Int_t nb = bot.GetNbinsX();
  if ( nt != nb ) return -1.;

  Float_t iT(0.);
  Float_t iB(0.);

  for ( Int_t i(1); i <= nt; i++ ) {
    Float_t vT = top.GetBinContent(i);
    Float_t n = bot.GetBinContent(i);
    iT += vT;
    iB += n;
    if ( vT > n ) return -2.;
    if ( n == 0 )  {
      top.SetBinContent(i,0.0);
      top.SetBinError(i,0.0);
      continue;
    }
    // sneak in 1/2 an event to distinguish perfect channels from channels w/ no data.
    if ( vT < 0.5 ) {
      vT = 0.5;
    }
    Float_t p = vT / n;
    Float_t q = 1. - p;
    Float_t npq = n*p*q;
    Float_t err = TMath::Sqrt(npq) / n;
    top.SetBinContent(i, inEffic ? q : p);
    top.SetBinError(i,err);
  }

  top.SetMinimum(0.);
  top.SetMaximum(1.);
  
  return fabs(iB) > 1e-9 ? iT / iB : 0.;
}

TestReport::TestReport(const char* dir, const char* prefix, 
		       const char* version, const char* emVersion,
		       const char*tkrCalibSerNo, const char* calCalibSerNo)
  : m_dir(dir), 
    m_prefix(prefix), 
    m_version(version), 
    m_emVersion(emVersion),
    m_tkrCalibSerNo(tkrCalibSerNo), 
    m_calCalibSerNo(calCalibSerNo), 
    m_outputFile(0), 
    m_mcFile(0), 
    m_mcTree(0),
    m_mcBranch(0), 
    m_mcEvent(0), 
    m_reconFile(0), 
    m_reconTree(0), 
    m_reconBranch(0), 
    m_reconEvent(0), 
    m_digiFile(0), 
    m_digiTree(0),
    m_digiBranch(0), 
    m_digiEvent(0), 
    m_trigger(0), 
    m_nBadEvts(0), 
    m_isLATTE(0),
    m_bay10Layer0SplitDefault(0),
    m_firstGroundID(0),
    m_lastGroundID(0),
    m_previousGroundID(0),
    m_counterGroundID(0),
    m_counterDataDiagramsEpu0(0),
    m_nbrDataGramsEpu0(0),
    m_nbrEventsDataGramsEpu0(0),
    m_firstDataGramEpu0(0),
    m_thisDataGramEpu0(0),
    m_previousDataGramEpu0(0),
    m_previousPreviousDataGramEpu0(0),
    m_endRunDataGramEpu0(0),
    m_fullDataGramEpu0(0),
    m_beginRunDataGramEpu0(0),
    m_counterDataDiagramsEpu1(0),
    m_nbrDataGramsEpu1(0),
    m_nbrEventsDataGramsEpu1(0),
    m_firstDataGramEpu1(0),
    m_thisDataGramEpu1(0),
    m_previousDataGramEpu1(0),
    m_previousPreviousDataGramEpu1(0),
    m_endRunDataGramEpu1(0),
    m_fullDataGramEpu1(0),
    m_beginRunDataGramEpu1(0),
    m_counterDataDiagramsEpu2(0),
    m_nbrDataGramsEpu2(0),
    m_nbrEventsDataGramsEpu2(0),
    m_firstDataGramEpu2(0),
    m_thisDataGramEpu2(0),
    m_previousDataGramEpu2(0),
    m_previousPreviousDataGramEpu2(0),
    m_endRunDataGramEpu2(0),
    m_fullDataGramEpu2(0),
    m_beginRunDataGramEpu2(0),
    m_counterCyclesSiu0(0),
    m_counterDataDiagramsSiu0(0),
    m_nbrDataGramsSiu0(0),
    m_nbrEventsDataGramsSiu0(0),
    m_firstDataGramSiu0(0),
    m_thisDataGramSiu0(0),
    m_previousDataGramSiu0(0),
    m_previousPreviousDataGramSiu0(0),
    m_endCountDataGramSiu0(0), 
    m_fullDataGramSiu0(0),
    m_beginRunDataGramSiu0(0),
    m_counterCyclesSiu1(0),
    m_counterDataDiagramsSiu1(0),
    m_nbrDataGramsSiu1(0),
    m_nbrEventsDataGramsSiu1(0),
    m_firstDataGramSiu1(0),
    m_thisDataGramSiu1(0),
    m_previousDataGramSiu1(0),
    m_previousPreviousDataGramSiu1(0),
    m_endCountDataGramSiu1(0),
    m_fullDataGramSiu1(0),
    m_beginRunDataGramSiu1(0),
    m_datagramGapsEPU0(0),
    m_datagramGapsEPU1(0),
    m_datagramGapsEPU2(0),
    m_datagramGapsSIU0(0),
    m_datagramGapsSIU1(0),
    m_nEvent(0),
    m_nEventNoPeriodic(0), 
    m_nbrPrescaled(0), 
    m_nbrDeadZone(0),
    m_nbrDiscarded(0), 
    m_deltaSequenceNbrEvents(0),
    m_nTkrTrigger(0), 
    m_nEventBadStrip(0), 
    m_nEventMoreStrip(0), 
    m_nEventSatTot(0), 
    m_nEventZeroTot(0), 
    m_nEvtInvalidTot(0), 
    m_nEvtOverlapTriggerTot(0),
    m_nEventBadTot(0), 
    m_startTime(0), 
    m_endTime(0),
    m_liveTime(0),
    m_elapsedTime(0),
    m_nbrEventsNormal(0),
    m_nbrEvents4Range(0),
    m_nbrEvents4RangeNonZS(0), 
    m_extendedCountersFlag(0),
    m_backwardsTimeTone(0),
    m_identicalTimeTones(0),
    m_nbrFlywheeling(0), 
    m_nbrIncomplete(0), 
    m_nbrMissingGps(0), 
    m_nbrMissingCpuPps(0), 
    m_nbrMissingLatPps(0), 
    m_nbrMissingTimeTone(0), 
    m_nDigi(0), 
    m_nAcdOddParityError(0), 
    m_nAcdHeaderParityError(0),
    m_eventBadEventSequence(0),
    m_eventBadTkrRecon(0),
    m_eventPacketError(0),
    m_eventTemError(0),
    m_eventTrgParityError(0),
    m_eventBadLdfStatus(0),
    m_eventGtrcPhase(0),
    m_eventGtfePhase (0),
    m_eventGtccFifo(0),
    m_eventGtccHdrParity(0),
    m_eventGtccWcParity(0),
    m_eventGtrcSummary(0),
    m_eventGtccDataParity(0),
    m_eventGtccTimeout(0),
    m_eventGcccError(0),
    m_eventGtccError(0),
    m_eventPhaseError(0),
    m_eventTimeoutError(0),
    m_eventIsPeriodic(0),
    m_AcdTileIdOnePMT(0), 
    m_AcdTileIdOneVeto(0),
    m_AcdHitMap(0), 
    m_AcdVetoMap(0),
    m_AcdPhaMapA(0), 
    m_AcdPhaMapB(0),
    m_AcdEfficMap(0),
    m_AcdInEfficMap(0),
    m_AcdMissMapTop(0), 
    m_AcdMissMapMinusX(0), 
    m_AcdMissMapMinusY(0),
    m_AcdMissMapPlusX(0), 
    m_AcdMissMapPlusY(0)
{ 
  // initialize ROOT
  if(gROOT == 0) {
    static TROOT g_root("report", "report");
  }
  gROOT->SetBatch();  // initialize ROOT
  if(gROOT == 0) {
    static TROOT g_root("report", "report");
  }
  gROOT->SetBatch();

  string f(dir);
  f += '/';
  f += prefix;
  f += "_report.doxygen";
  m_report = new ofstream(f.c_str());
  m_report->precision(2);

  string r(dir);
  r += '/';
  r += prefix;
  r += "_hist.root";
  m_outputFile = new TFile(r.c_str(), "RECREATE");

  std::fill((int*) m_nFec, ((int*) m_nFec)+g_nTower*g_nLayer*g_nView*2, 12);

  m_trigger = new TH1F("trigger", "Trigger distribution", 32, -0.5, 31.5);
  HistAttribute att("Trigger word", "Number of events");
  setHistParameters(m_trigger, att);

  m_condSummary = new TH1F("condSummary", "Condition summary distribution", 32, -0.5, 31.5);
  att.m_xTitle = "GEM Condition summary word";
  setHistParameters(m_condSummary, att);

  m_tkrPerTower = new TH1F("tkrPerTower","Number of TKR triggers per tower",32,0,16);
  att.set("Tower number","Number of TKR triggers");
  setHistParameters(m_tkrPerTower, att);

  m_calLoPerTower = new TH1F("calLoPerTower","Number of CAL LO triggers per tower",32,0,16);
  att.set("Tower number","Number of CAL LO triggers");
  setHistParameters(m_calLoPerTower, att);

  m_calHiPerTower = new TH1F("calHiPerTower","Number of CAL Hi triggers per tower",32,0,16);
  att.set("Tower number","Number of CAL HI triggers");
  setHistParameters(m_calHiPerTower, att);

  for(int i = 0; i != 7; ++i) {
    m_nEventDigi[i] = 0;
  }

  m_nHit2D = new TH2F("nHit2D", "nhits distribution", 36, -0.5, 35.5, 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_nHit2D, att);

  m_nZeroHit2D = new TH2F("nZeroHit2D", "no. of events with zero strip hit in a particular TKR plane", 72, 0, 36, 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_nZeroHit2D, att);

  for(int i = 0; i != g_nTower; ++i) {
    char name[] = "nHitss00";
    sprintf(name, "nHitss%02d", i);
    char xTitle[] = "Number of strip hits in tower 00";
    sprintf(xTitle, "Number of strip hits in tower %02d", i);
    m_nHit[i] = new TH1F(name, xTitle, 20, 0., 20.);
    att.set(xTitle, "Number of events", 0.1, 0.07, 0.6, 0.4);
    setHistParameters(m_nHit[i], att);
 
    char name1[] = "nLayer00";
    sprintf(name1, "nLayer%02d", i);
    char xTitle1[] = "Number of hit planes in tower 00";
    sprintf(xTitle1, "Number of hit planes in tower %02d", i);
    m_nLayer[i] = new TH1F(name1, xTitle1, 37, -0.5, 36.5);
    att.set(xTitle1, "Number of events", 0.1, 0.07, 0.6, 0.4);
    setHistParameters(m_nLayer[i], att);

    m_nTkrEvent[i] = 0;
    m_nTkrBadEvent[i] = 0;

    for(int j = 0; j != g_nPlane; ++j) {
      m_nHitPlane[i][j] = 0;
      m_nEvtPlane[i][j] = 0;
      m_nEvtHitPlane[i][j][0] = 0;
      m_nEvtHitPlane[i][j][1] = 0;
      for(int k = 0; k != 2; ++k) {
	char name[] = "tot00000";
	sprintf(name, "tot%02d%02d%1d", i, j, k);
	m_tot[i][j][k] = new TH1F(name, name, g_satTot, 0, g_satTot);
      }
    }

    for(int j = 0; j != g_nCalLayer; ++j) {
      m_calEneLayer[i][j] = 0;
      m_nCalEneLayer[i][j] = 0;
      m_nCalHit[i][j] = 0;
      m_nEvtCalHit[i][j] = 0;
    }
  }

  for(int i = 0; i != enums::number_of_trigger_bits; ++i) {
    m_nEvtGltTrigger[i] = 0; 
  }

  for(int i = 0; i != enums::GEM_offset; ++i) {
    m_nEvtGemTrigger[i] = 0;
  }

  m_nCalHit2D = new TH2F("nCalHit2D", "nhits distribution", 8, -0.5, 7.5, 16, -0.5, 15.5);
  att.set("Layer", "Tower");
  setHistParameters(m_nCalHit2D, att);

  m_nZeroCalHit2D = new TH2F("nZeroCalHit2D", "ratio of events with no hit in a particular CAL layer", 8, -0.5, 7.5, 16, -0.5, 15.5);
  att.set("Layer", "Tower");
  setHistParameters(m_nZeroCalHit2D, att);

  m_totZero2D = new TH2F("totZero2D", "Zerot tot distribution", 72, 0., 36., 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_totZero2D, att);

  m_totNoise2D = new TH2F("totNoise2D", "Noise tot distribution", 72, 0., 36., 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_totNoise2D, att);

  m_totSat2D = new TH2F("totSat2D", "Tot distribution", 72, 0, 36, 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_totSat2D, att);

  m_totAve2D = new TH2F("totAve2D", "Tot distribution", 72, 0, 36, 16, -0.5, 15.5);
  att.set("Plane", "Tower");
  setHistParameters(m_totAve2D, att);

  m_nTkrTrack = new TH1F("nTkrTrack", "Number of reconstructed tracks", 11, -0.5, 10.5);
  att.set("Number of reconstructed tracks", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_nTkrTrack, att);

  m_reconDirXY = new TH2F("reconDirXY", "Reconstructed dir xy distribution", 100, -1, 1, 100, -1, 1);
  att.set("Reconstructed x direction", "Reconstructed Y direction");
  setHistParameters(m_reconDirXY, att);

  m_reconDirZ = new TH1F("reconDirZ", "Reconstructed dir Z distribution", 100, -1, 0.);
  att.set("Reconstructed z direction", "Number of events");
  setHistParameters(m_reconDirZ, att);

  m_reconPosXY = new TH2F("reconPosXY", "Reconstructed dir XY position", 100, -1000, 1000, 100, -1000, 1000);
  att.set("Reconstructed x position", "Reconstructed y position");
  att.m_canRebin = false;
  att.m_use2DStat = 1;
  setHistParameters(m_reconPosXY, att);

  m_reconPosZ = new TH1F("reconPosZ", "Reconstructed dir Z position", 100, -100, 750.);
  att.set("Reconstructed z position", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_reconPosZ, att);

  m_reconEne = new TH1F("reconEne", "Reconstructed energy", 100, 0., 400.);
  att.set("Reconstructed energy(MeV)", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_reconEne, att);

  m_calSumEne = new TH1F("calSumEne", "Cal energy", 100, 0., 400.);
  att.set("Sum of crystal crystal energies in the CAL cluster(MeV).", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_calSumEne, att);

  m_calEneLayer2D = new TH2F("calEneLayer", "Energy in each CAL layer(MeV)", 8, -0.5, 7.5, 16, -0.5, 15.5);
  att.set("Layer", "Tower");
  setHistParameters(m_calEneLayer2D, att);

  m_zeroCalEneLayer2D = new TH2F("zeroCalEneLayer", "Ratio of events with zero energy measured in a particular CAL layer(MeV)", 8, -0.5, 7.5, 16, -0.5, 15.5);
  att.set("Layer", "Tower");
  setHistParameters(m_zeroCalEneLayer2D, att);

  m_epu = new TH1F("epu","Crate Number (No crate - Epu0 - Epu1 - Epu2 - Siu0 - Siu1 - Aux) - ",22,-1,10);
  att.set("Crate number (Epu0 - Epu1 - Epu2 - Siu0 - Siu1 - Aux)","Number of events");
  setHistParameters(m_epu,att);

  m_datagramsEPU0 = new TH1F("datagramsEPU0","Number of events per datagram - EPU0",50,-1,100);
  att.set("Number of events per datagram - EPU0","Number of events");
  setHistParameters(m_datagramsEPU0,att);

  m_datagramsEPU1 = new TH1F("datagramsEPU1","Number of events per datagram - EPU1",50,-1,100);
  att.set("Number of events per datagram - EPU1","Number of events");
  setHistParameters(m_datagramsEPU1,att);

  m_datagramsEPU2 = new TH1F("datagramsEPU2","Number of events per datagram - EPU2",50,-1,100);
  att.set("Number of events per datagram - EPU2","Number of events");
  setHistParameters(m_datagramsEPU2,att);

  m_datagramsSIU0 = new TH1F("datagramsSIU0","Number of events per datagram - SIU0",50,-1,100);
  att.set("Number of events per datagram - SIU0","Number of events");
  setHistParameters(m_datagramsSIU0,att);

  m_datagramsSIU1 = new TH1F("datagramsSIU1","Number of events per datagram - SIU1",50,-1,100);
  att.set("Number of events per datagram - SIU1","Number of events");
  setHistParameters(m_datagramsSIU1,att);

  m_deltaTimeDGCTEvtEPU0 = new TH1F("deltaTimeDGCTEvtEPU0","CCSDS Secondary Header Time minus the event time for events from EPU0 [seconds]",50,0.0,1.0);
  att.set("CCSDS time minus the event time for events from EPU0 [seconds]","Number of events");
  setHistParameters(m_deltaTimeDGCTEvtEPU0,att);

  m_deltaTimeDGCTEvtEPU1 = new TH1F("deltaTimeDGCTEvtEPU1","CCSDS Secondary Header Time minus the event time for events from EPU1 [seconds]",50,0.0,1.0);
  att.set("CCSDS Time minus the event time for events from EPU1 [seconds]","Number of events");
  setHistParameters(m_deltaTimeDGCTEvtEPU1,att);

  m_deltaTimeDGCTEvtEPU2 = new TH1F("deltaTimeDGCTEvtEPU2","CCSDS Secondary Header Time minus the event time for events from EPU2 [seconds]",50,0.0,1.0);
  att.set("CCSDS Time minus the event time for events from EPU2 [seconds]","Number of events");
  setHistParameters(m_deltaTimeDGCTEvtEPU2,att);

  m_deltaTimeDGCTEvtSIU0 = new TH1F("deltaTimeDGCTEvtSIU0","CCSDS Secondary Header Time minus the event time for events from SIU0 [seconds]",50,0.0,1.0);
  att.set("CCSDS Time minus the event time for events from SIU0 [seconds]","Number of events");
  setHistParameters(m_deltaTimeDGCTEvtSIU0,att);

  m_deltaTimeDGCTEvtSIU1 = new TH1F("deltaTimeDGCTEvtSIU1","CCSDS Secondary Header Time minus the event time for events from SIU1 [seconds]",50,0.0,1.0);
  att.set("CCSDS Time minus the event time for events from SIU1 [seconds]","Number of events");
  setHistParameters(m_deltaTimeDGCTEvtSIU1,att);

  m_deltaEventIDEPU0 = new TH1F("deltaEventIDEPU0","Delta Event ID For Successive Events From EPU0",50,0.0,3.0);
  att.set("Delta Event ID For Successive Events From EPU0","Number of events");
  setHistParameters(m_deltaEventIDEPU0,att);

  m_deltaEventIDEPU1 = new TH1F("deltaEventIDEPU1","Delta Event ID For Successive Events From EPU1",50,0.0,3.0);
  att.set("Delta Event ID For Successive Events From EPU1","Number of events");
  setHistParameters(m_deltaEventIDEPU1,att);

  m_deltaEventIDEPU2 = new TH1F("deltaEventIDEPU2","Delta Event ID For Successive Events From EPU2",50,0.0,3.0);
  att.set("Delta Event ID For Successive Events From EPU2","Number of events");
  setHistParameters(m_deltaEventIDEPU2,att);

  m_deltaEventIDSIU0 = new TH1F("deltaEventIDSIU0","Delta Event ID For Successive Events From SIU0",50,0.0,3.0);
  att.set("Delta Event ID For Successive Events From SIU0","Number of events");
  setHistParameters(m_deltaEventIDSIU0,att);

  m_deltaEventIDSIU1 = new TH1F("deltaEventIDSIU1","Delta Event ID For Successive Events From SIU1",50,0.0,3.0);
  att.set("Delta Event ID For Successive Events From SIU1","Number of events");
  setHistParameters(m_deltaEventIDSIU1,att);



  m_triggerRate = new TH1F("triggerRate","Trigger rate for 30 equally spaced time intervals",30,0,30);
  att.set("Trigger rate for 30 time intervals","Trigger rate [Hz]");
  setHistParameters(m_triggerRate,att);

  m_triggerLivetimeRate = new TH1F("triggerLivetimeRate","Livetime corrected trigger rate for 30 equally spaced time intervals",30,0,30);
  att.set("Livetime corrected trigger rate for 30 time intervals","Livetime corrected trigger rate [Hz]");
  setHistParameters(m_triggerLivetimeRate,att);

  m_livetimeRate = new TH1F("livetimeRate","Livetime for 30 equally spaced time intervals",30,0,30);
  att.set("Livetime for 30 time intervals","Livetime [%]");
  setHistParameters(m_livetimeRate,att);

  m_deadzoneRate = new TH1F("deadzoneRate","Deadzone rate for 30 equally spaced time intervals",30,0,30);
  att.set("Deadzone rate for 30 time intervals","Deadzone rate [Hz]");
  setHistParameters(m_deadzoneRate,att);

  m_discardedRate = new TH1F("discardedRate","Discarded rate for 30 equally spaced time intervals",30,0,30);
  att.set("Discarded rate for 30 time intervals","Discarded rate [Hz]");
  setHistParameters(m_discardedRate,att);

  m_gemDiscarded = new TH1F("gemDiscarded","Number of GEM discarded events between two successive read out events",40,0,20);
  att.set("Number of GEM discarded events","Number of events");
  setHistParameters(m_gemDiscarded,att);

  m_gemDiscardedTime = new TH1F("gemDiscardedTime","Time between the previous read out event and the last discarded event in system clock ticks (nominally 50 ns)",100,0,530);
  att.set("Time of last discarded event (nominally 50 ns ticks)","Number of events");
  setHistParameters(m_gemDiscardedTime,att);

  m_gemDeadzone = new TH1F("gemDeadzone","Number of GEM deadzone events between two successive read out events",40,0,20);
  att.set("Number of GEM deadzone events","Number of events");
  setHistParameters(m_gemDeadzone,att);

  m_timeInterval = new TH1F("timeInterval", "Time interval between adjacent event in mill second", 100, 0., 3.);
  att.set("Time interval between adjacent events (ms)", "Number of events");
  setHistParameters(m_timeInterval, att);

  m_timeIntervalCut = new TH1F("timeIntervalCut", "Time interval between adjacent event in millseconds with a cut of 1 millisecond", 100, 0., 1.);
  att.set("Time interval between adjacent events (ms)", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_timeIntervalCut, att);

  m_timeIntervalGem = new TH1F("timeIntervalGem", "Time interval between adjacent events in milliseconds as measured by the GEM", 100, 0., 3.);
  att.set("Time interval between adjacent events (ms)", "Number of events");
  setHistParameters(m_timeIntervalGem, att);

  m_timeIntervalGemZoom = new TH1F("timeIntervalGemZoom", "Time interval between adjacent events in system clock ticks (nominally 50 ns) as measured by the GEM with a cut of 1500 ticks", 100, 0., 1500);
  att.set("Time interval between adjacent events (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_timeIntervalGemZoom, att);

  m_deltaWindowOpenTime = new TH1F("deltaWindowOpenTime", "Delta window open time in milliseconds as measured by the GEM", 100, 0., 3.);
  att.set("Delta window open time (ms)", "Number of events");
  setHistParameters(m_deltaWindowOpenTime, att);

  m_deltaWindowOpenTimeZoom = new TH1F("deltaWindowOpenTimeZoom", "Delta window open time in system clock ticks (nominally 50 ns) as measured by the GEM with a cut of 1500 ticks", 100, 0., 1500);
  att.set("Delta window open time (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_deltaWindowOpenTimeZoom, att);

  m_tick20MHzDeviation = new TH1F("tick20MHzDeviation", "Number of ticks between successive 1-PPS - Deviation from 20 MHz", 100, 0., 1500);
  att.set("Number of ticks between successive 1-PPS", "Number of events");
  setHistParameters(m_tick20MHzDeviation, att);

  m_tick20MHzDeviationZoom = new TH1F("tick20MHzDeviationZoom", "Number of ticks between successive 1-PPS - Deviation from 20 MHz - Zoomed", 100, 0., 200);
  att.set("Number of ticks between successive 1-PPS - Zoom", "Number of events");
  setHistParameters(m_tick20MHzDeviationZoom, att);

  m_timeIntervalElapsed = new TH1F("timeIntervalElapsed", "Elapsed time between adjacent events in milliseconds", 100, 0., 5.);
  att.set("Elapsed time between adjacent events (ms)", "Number of events");
  setHistParameters(m_timeIntervalElapsed, att);

  m_condArrivalTimeTKR = new TH1F("condArrivalTimeTKR", "Conditions arrival time for TKR", 32, 0, 32);
  att.set("Conditions arrival time for TKR (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_condArrivalTimeTKR, att);

  m_condArrivalTimeROI = new TH1F("condArrivalTimeROI", "Conditions arrival time for ROI", 32, 0, 32);
  att.set("Conditions arrival time for ROI (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_condArrivalTimeROI, att);

  m_condArrivalTimeCalLo = new TH1F("condArrivalTimeCalLo", "Conditions arrival time for CAL LO", 32, 0, 32);
  att.set("Conditions arrival time for CAL LO (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_condArrivalTimeCalLo, att);

  m_condArrivalTimeCalHi = new TH1F("condArrivalTimeCalHi", "Conditions arrival time for CalHi", 32, 0, 32);
  att.set("Conditions arrival time for CAL HI (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_condArrivalTimeCalHi, att);

  m_condArrivalTimeCNO = new TH1F("condArrivalTimeCNO", "Conditions arrival time for CNO", 32, 0, 32);
  att.set("Conditions arrival time for CNO (nominally 50 ns ticks)", "Number of events");
  setHistParameters(m_condArrivalTimeCNO, att);


  m_alignCalTkr = new TH1F("alignCalTkr", "Distance between the reconstructed CAL cluster XY coordinates and the XY coordinates extrapolated from TKR", 50, 0., 100.);
  att.set("Difference (mm)", "Number of events");
  att.m_canRebin = false;
  setHistParameters(m_alignCalTkr, att);

  m_nAcdDigis = new TH1F("nAcdDigis","Number of ACD digis",100,-0.5,99.5);
  att.set("Number of ACD digis","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_nAcdDigis,att);

  m_AcdTileIdOnePMT = new TH1F("AcdTileIdOnePMT","ACD tile ID for single PMT digis",128,-0.5,127.5);
  att.set("ACD GEM ID","Fraction of events w/ only 1 PMT fired");
  att.m_canRebin = false;
  setHistParameters(m_AcdTileIdOnePMT,att);

  m_AcdTileIdOneVeto = new TH1F("AcdTileIdOneVeto","ACD tile ID for single Veto digis",128,-0.5,127.5);
  att.set("ACD GEM ID","Fraction of events w/ only 1 Veto signal");
  att.m_canRebin = false;
  setHistParameters(m_AcdTileIdOneVeto,att);

  m_AcdHitMap = new TH1F("AcdHitMap","ACD GEM ID for Hits",128,-0.5,127.5);
  att.set("ACD GEM ID","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_AcdHitMap,att);
 
  m_AcdVetoMap = new TH1F("AcdVetoMap","ACD GEM ID for Vetos",128,-0.5,127.5);
  att.set("ACD GEM ID","Fraction of events above Veto threshold");
  att.m_canRebin = false;
  setHistParameters(m_AcdVetoMap,att);
  
  m_AcdPhaMapA = new TH2F("AcdPhaMapA","ACD GEM ID vs PHA for A PMT",128,-0.5,127.5,256,0,4096);
  att.set("ACD GEM ID","PHA counts");
  att.m_canRebin = false;
  setHistParameters(m_AcdPhaMapA,att);

  m_AcdPhaMapB = new TH2F("AcdPhaMapB","ACD GEM ID vs PHA for B PMT",128,-0.5,127.5,256,0,4096);
  att.set("ACD GEM ID","PHA counts");
  att.m_canRebin = false;
  setHistParameters(m_AcdPhaMapB,att);

  m_AcdGarcGafeHitMap = new TH2F("AcdGarcGafeHitMap","GARC:GAFE for all hits",18,-0.5,17.5,12,-0.5,11.5);
  att.set("GAFE","GARC");
  att.m_canRebin = false;
  setHistParameters(m_AcdGarcGafeHitMap,att);

  m_AcdGarcGafeVetoMap = new TH2F("AcdGarcGafeVetoMap","GARC:GAFE for all vetos",18,-0.5,17.5,12,-0.5,11.5);
  att.set("GAFE","GARC");
  att.m_canRebin = false;
  setHistParameters(m_AcdGarcGafeVetoMap,att);

  m_AcdGemVetoMap = new TH1F("AcdGemVetoMap","Number of GEM vetos by Tile ID",128,-0.5,127.5); 
  att.set("ACD GEM ID","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_AcdGemVetoMap,att);

  m_AcdGemRoiMap = new TH1F("AcdGemRoiMap","Number of GEM ROI by Tower",16,-0.5,15.5); 
  att.set("Tower","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_AcdGemRoiMap,att);

  m_AcdGemCnoMap = new TH1F("AcdGemCnoMap","Number of GEM CNO by GARC",12,-0.5,11.5); 
  att.set("GARC","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_AcdGemCnoMap,att);

  m_AcdEfficMap = new TH1F("AcdEfficMap","ACD Effic by Tile ID",128,-0.5,127.5); 
  att.set("ACD GEM ID","Number of events");
  att.m_canRebin = false;
  setHistParameters(m_AcdEfficMap,att);

  m_AcdInEfficMap = new TH1F("AcdInEfficMap","ACD InEffic by Tile ID",128,-0.5,127.5); 
  att.set("ACD GEM ID","Fraction on expected hits that are missing");
  att.m_canRebin = false;
  setHistParameters(m_AcdInEfficMap,att);
  
  m_AcdMissMapTop = new TH2F("AcdMissMapTop","ACD X-Y location for misses",200,-800.,800.,200,-800.,800.);
  att.set("X (mm)","Y (mm)");
  att.m_canRebin = false;
  setHistParameters(m_AcdMissMapTop,att);

  m_AcdMissMapMinusX = new TH2F("AcdMissMapMinusX","ACD Y-Z location for misses",200,-800.,800.,200,0.,800.);
  att.set("Y (mm)","Z (mm)");
  att.m_canRebin = false;
  setHistParameters(m_AcdMissMapMinusX,att);

  m_AcdMissMapMinusY = new TH2F("AcdMissMapMinusY","ACD X-Z location for misses",200,-800.,800.,200,0.,800.);
  att.set("X (mm)","Z (mm)");
  att.m_canRebin = false;
  setHistParameters(m_AcdMissMapMinusY,att);

  m_AcdMissMapPlusX = new TH2F("AcdMissMapPlusX","ACD Y-Z location for misses",200,-800.,800.,200,0.,800.);
  att.set("Y (mm)","Z (mm)");
  att.m_canRebin = false;
  setHistParameters(m_AcdMissMapPlusX,att);

  m_AcdMissMapPlusY = new TH2F("AcdMissMapPlusY","ACD X-Z location for misses",200,-800.,800.,200,0.,800.);
  att.set("X (mm)","Z (mm)");
  att.m_canRebin = false;
  setHistParameters(m_AcdMissMapPlusY,att);

  m_AcdMipMapA = new TH2F("AcdMipMapA","ACD GEM ID vs MIP for A PMT",128,-0.5,127.5,80,0.,5.);
  att.set("ACD GEM ID","MIPs");
  att.m_canRebin = false;
  setHistParameters(m_AcdMipMapA,att);

  m_AcdMipMapB = new TH2F("AcdMipMapB","ACD GEM ID vs MIP for B PMT",128,-0.5,127.5,80,0.,5.);
  att.set("ACD GEM ID","MIPs");
  att.m_canRebin = false;
  setHistParameters(m_AcdMipMapB,att);
  
}

TestReport::~TestReport()
{
  if(m_outputFile) {
    m_outputFile->cd();
    m_outputFile->Write(0, TObject::kOverwrite);
    m_outputFile->Close();

    // when the file is deleted, all trees and histograms associated with it
    // also get deleted. So they should not be manually deleted again
    delete m_outputFile;
  }

  delete m_mcFile;
  delete m_digiFile;
  delete m_reconFile;
  delete m_report;
}

void TestReport::setHistParameters(TH1* h, const HistAttribute& att)
{
  assert(h != 0);

  h->SetXTitle(att.m_xTitle.c_str());
  h->SetYTitle(att.m_yTitle.c_str());
  h->SetTitle("");
  h->SetFillColor(2);
  h->SetLineWidth(1);
  h->SetLabelSize(att.m_axisLabelSize, "X");
  h->SetLabelSize(att.m_axisLabelSize, "Y");
  h->GetXaxis()->SetTitleSize(att.m_axisTitleSize);
  h->GetYaxis()->SetTitleSize(att.m_axisTitleSize);
  h->GetXaxis()->SetTitleOffset(att.m_xTitleOffset);
  h->GetYaxis()->SetTitleOffset(att.m_yTitleOffset);

  if(att.m_canRebin) h->SetBit(TH1::kCanRebin); 

  if(TH2F* h2 = dynamic_cast<TH2F*>(h)) {
    if (att.m_use2DStat) {
      h2->SetStats(kTRUE);
    } else {
      h2->SetStats(kFALSE);
    }
  }
}

void TestReport::setGraphParameters(TGraph* h, const GraphAttribute& att)
{
  assert(h != 0);

  h->GetXaxis()->SetTitle(att.m_xTitle.c_str());
  h->GetYaxis()->SetTitle(att.m_yTitle.c_str());
  h->SetTitle("");
  h->GetXaxis()->SetLabelSize(att.m_axisLabelSize);
  h->GetYaxis()->SetLabelSize(att.m_axisLabelSize);
  h->GetXaxis()->SetTitleSize(att.m_axisTitleSize);
  h->GetYaxis()->SetTitleSize(att.m_axisTitleSize);
  h->GetXaxis()->SetTitleOffset(att.m_xTitleOffset);
  h->GetYaxis()->SetTitleOffset(att.m_yTitleOffset);

  h->SetMarkerSize(att.m_markerSize);
}


UShort_t TestReport::getGemId(UInt_t id) {
  UInt_t face = id / 100;
  UInt_t row = (id % 100 ) / 10;
  UInt_t col = (id % 10 );
  switch ( face ) {
  case 0:  return 64 + 5*row + col;  // top:      64 - 89
  case 1:  return 32 + 5*row + col;  // -x side:  32 - 47
  case 2:  return      5*row + col;  // -y side    0 - 15
  case 3:  return 48 + 5*row + col;  // +x side   48 - 63
  case 4:  return 16 + 5*row + col;  // +y side   16 - 31
  case 5:                                   // x ribbons 96 - 99
    return 96 + col;
  case 6:                                   // y ribbons 100- 103
    return 100 + col;
  }
  return 0xFFFF;
}

void TestReport::analyzeTrees(const char* mcFileName="mc.root",
			      const char* digiFileName="digi.root",
			      const char* reconFileName="recon.root")
{
  //open 3 root files

  if(mcFileName) m_mcFile = new TFile(mcFileName, "READ");
  // check whether file exists
  if(m_mcFile && m_mcFile->IsZombie()) m_mcFile = 0;
  if(m_mcFile) {
    m_mcTree = (TTree*) m_mcFile->Get("Mc");
    m_mcEvent = 0;  
    m_mcBranch = m_mcTree->GetBranch("McEvent");
    // what is stored in the root tree is actually a pointer rather than
    // mc event
    m_mcBranch->SetAddress(&m_mcEvent);
  }

  if(reconFileName) m_reconFile = new TFile(reconFileName, "READ");
  if(m_reconFile && m_reconFile->IsZombie()) m_reconFile = 0;
  if(m_reconFile) {
    m_reconTree = (TTree*) m_reconFile->Get("Recon");
    m_reconEvent = 0;
    m_reconBranch = m_reconTree->GetBranch("ReconEvent");
    m_reconBranch->SetAddress(&m_reconEvent);
  }

  if(digiFileName) m_digiFile = new TFile(digiFileName, "READ");
  if(m_digiFile && m_digiFile->IsZombie()) m_digiFile = 0;
  if(m_digiFile) {
    m_digiTree = (TTree*) m_digiFile->Get("Digi");
    m_digiEvent = 0;
    m_digiBranch = m_digiTree->GetBranch("DigiEvent");
    m_digiBranch->SetAddress(&m_digiEvent);
  }

  // Make sure we have the same number of events in the input files:
  m_nEvent   = -1;
  int nMc    = -1;
  int nRecon = -1;
  int nDigi  = -1;

  if(m_mcFile) {
    nMc = (int) m_mcTree->GetEntries();
    std::cout << "No of events in " << mcFileName << " : " << nMc << endl;
    m_nEvent = nMc;
  }
  if(m_reconFile) {
    nRecon = (int) m_reconTree->GetEntries();
    std::cout << "No of events in " << reconFileName << " : " << nRecon << endl;
    m_nEvent = nRecon;
  }
  if(m_digiFile) {
    nDigi = (int) m_digiTree->GetEntries();
    std::cout << "No of events in " << digiFileName << " : " << nDigi << endl;
    m_nEvent = nDigi;
  }

  // Want same number of event sin files:
  if (nMc != -1) {
    assert (nMc == m_nEvent);
  }
  if (nRecon != -1) {
    assert (nRecon == m_nEvent);
  }
  if (nDigi != -1) {
    assert (nDigi == m_nEvent);
  }

      
 

  // For testing: awb
  //int nEvent = 50000;
  //m_nEvent = nEvent;

  // List of datagrams:
  std::list<int> listDataGramsEpu0;
  std::list<int> listDataGramsEpu1;
  std::list<int> listDataGramsEpu2;
  std::list<int> listDataGramsSiu0;
  std::list<int> listDataGramsSiu1;

  //
  // For the trigger/deadzone rate intervals:
  //
  ULong64_t elapsedTimeFirst = 0;
  ULong64_t elapsedTimeLast  = 0;

  Long64_t deltaTimeInterval = 1;

  Int_t nbrTimeIntervals = 30;
  Int_t intervalCounter  = 1;

  Int_t eventCounter = 0;

  ULong64_t livetimeFirst = 0;
  ULong64_t livetimeLast = 0;


  ULong64_t deadzoneCounter  = 0;
  ULong64_t discardedCounter = 0;

  ULong64_t previousDeadZone  = 0;
  ULong64_t previousDiscarded = 0;

  ULong64_t previousElapsed   = 0;
  ULong64_t previousLiveTime  = 0;
  ULong64_t previousSequence  = 0;
  ULong64_t previousPrescaled = 0;


  // Time tones going backwards?
  UInt_t previousTimeToneCurrentSec  = 0;
  UInt_t previousTimeTonePreviousSec = 0;

  UInt_t previousTimeToneCurrentTicks  = 0;
  UInt_t previousTimeTonePreviousTicks = 0;


  //
  // Time tone counters and flags:
  //
  m_nbrFlywheeling     = 0;
  m_nbrIncomplete      = 0;
  m_nbrMissingGps      = 0;
  m_nbrMissingCpuPps   = 0;
  m_nbrMissingLatPps   = 0;
  m_nbrMissingTimeTone = 0;  

  UInt_t firstFlywheeling;
  UInt_t lastFlywheeling;

  // Extended counter flag:
  m_extendedCountersFlag = 0;

  // Time tone going backwards flag:
  m_backwardsTimeTone = 0;

  // Current and previous time tones identical?
  m_identicalTimeTones = 0;  

  // Ground ID changes?
  m_counterGroundID = 0;

  // Number of events per datagram:
  int nbrEventsDatagram[5];
  int idDatagram[5];

  for (int iLoop = 0; iLoop < 5; ++iLoop) {
    nbrEventsDatagram[iLoop] = 0;
    idDatagram[iLoop]        = 0;
  }


  // Delta Event ID doe each CPU:
  ULong64_t previousEventIDEPU0 = 0;
  ULong64_t currentEventIDEPU0  = 0;

  ULong64_t previousEventIDEPU1 = 0;
  ULong64_t currentEventIDEPU1  = 0;

  ULong64_t previousEventIDEPU2 = 0;
  ULong64_t currentEventIDEPU2  = 0;

  ULong64_t previousEventIDSIU0 = 0;
  ULong64_t currentEventIDSIU0  = 0;

  ULong64_t previousEventIDSIU1 = 0;
  ULong64_t currentEventIDSIU1  = 0;


  // Look at first and last event:
  if(m_digiFile) {
    m_digiBranch->GetEntry(0);
    ULong64_t gemSequenceFirst  = m_digiEvent->getMetaEvent().scalers().sequence();
    ULong64_t gemPrescaledFirst = m_digiEvent->getMetaEvent().scalers().prescaled();
    ULong64_t gemDeadZoneFirst  = m_digiEvent->getMetaEvent().scalers().deadzone();
    ULong64_t gemDiscardedFirst = m_digiEvent->getMetaEvent().scalers().discarded();
    ULong64_t liveTimeFirst     = m_digiEvent->getMetaEvent().scalers().livetime();
    elapsedTimeFirst            = m_digiEvent->getMetaEvent().scalers().elapsed();
    firstFlywheeling            = m_digiEvent->getMetaEvent().time().current().flywheeling();
    m_firstGroundID             = m_digiEvent->getMetaEvent().run().id();
    m_digiEvent->Clear();


    m_digiBranch->GetEntry(m_nEvent-1);
    ULong64_t gemSequenceLast  = m_digiEvent->getMetaEvent().scalers().sequence();
    ULong64_t gemPrescaledLast = m_digiEvent->getMetaEvent().scalers().prescaled();
    ULong64_t gemDeadZoneLast  = m_digiEvent->getMetaEvent().scalers().deadzone();
    ULong64_t gemDiscardedLast = m_digiEvent->getMetaEvent().scalers().discarded();
    ULong64_t liveTimeLast     = m_digiEvent->getMetaEvent().scalers().livetime();
    elapsedTimeLast            = m_digiEvent->getMetaEvent().scalers().elapsed();
    lastFlywheeling            = m_digiEvent->getMetaEvent().time().current().flywheeling();
    m_lastGroundID             = m_digiEvent->getMetaEvent().run().id();
    m_digiEvent->Clear();

    // Flywheeling:
    m_nbrFlywheeling = lastFlywheeling - firstFlywheeling;



    // Context information:
    m_nbrPrescaled           = gemPrescaledLast - gemPrescaledFirst;
    m_nbrDeadZone            = gemDeadZoneLast  - gemDeadZoneFirst;
    m_nbrDiscarded           = gemDiscardedLast - gemDiscardedFirst;
    m_deltaSequenceNbrEvents = m_nEvent - (gemSequenceLast-gemSequenceFirst + 1);

    // Livetime: 
    Double_t tmpElapsedTime = elapsedTimeLast - elapsedTimeFirst;
    Double_t tmpLiveTime    = liveTimeLast    - liveTimeFirst;
    if (tmpElapsedTime != 0.0) {
      m_liveTime = tmpLiveTime/tmpElapsedTime;
    } else {
      m_liveTime = -1.0;
    }
    m_elapsedTime = elapsedTimeLast - elapsedTimeFirst;

    // For trigger rates in time intervals:
    deltaTimeInterval = (elapsedTimeLast - elapsedTimeFirst)/nbrTimeIntervals;
  }



  // For GEM discarded and deadzone events:
  ULong64_t previousGemDiscarded = 0;
  ULong64_t previousGemDeadzone  = 0;


  // Datagrams:
  m_previousDataGramEpu0         = 0;
  m_previousPreviousDataGramEpu0 = 0;
  m_previousDataGramEpu1         = 0;
  m_previousPreviousDataGramEpu1 = 0;
  m_previousDataGramEpu2         = 0;
  m_previousPreviousDataGramEpu2 = 0;
  m_previousDataGramSiu0         = 0;
  m_previousPreviousDataGramSiu1 = 0;
  m_previousDataGramSiu1         = 0;
  m_previousPreviousDataGramSiu1 = 0;

  int firstDatagramEventEpu0 = -1;
  int firstDatagramEventEpu1 = -1;
  int firstDatagramEventEpu2 = -1;
  int firstDatagramEventSiu0 = -1;
  int firstDatagramEventSiu1 = -1;

  int lastDatagramEventEpu0 = -1;
  int lastDatagramEventEpu1 = -1;
  int lastDatagramEventEpu2 = -1;
  int lastDatagramEventSiu0 = -1;
  int lastDatagramEventSiu1 = -1;

  m_nbrEventsDataGramsEpu0 = 0;
  m_nbrEventsDataGramsEpu1 = 0;
  m_nbrEventsDataGramsEpu2 = 0;
  m_nbrEventsDataGramsSiu0 = 0;
  m_nbrEventsDataGramsSiu1 = 0;

  int previousDatagramGapsEPU0 = 0;
  int previousDatagramGapsEPU1 = 0;
  int previousDatagramGapsEPU2 = 0;
  int previousDatagramGapsSIU0 = 0;
  int previousDatagramGapsSIU1 = 0;


  // Loop over events:
  for(int iEvent = 0; iEvent != m_nEvent; ++iEvent) {

    //if ( iEvent % 1000 == 0 ) {
    if ( iEvent % 1000 == 0 ) {
      std::cout << "Event number " << iEvent << std::endl;
    }

    // Cleanup:
    if (m_mcEvent)    m_mcEvent->Clear();
    if (m_digiEvent)  m_digiEvent->Clear();
    if (m_reconEvent) m_reconEvent->Clear();

    // Analyze MC file:
    if (m_mcFile) {
      m_mcBranch->GetEntry(iEvent);
      analyzeMcTree();
    }

    // Analyze digi file:
    if (m_digiFile) {
      m_digiBranch->GetEntry(iEvent);

      analyzeDigiTree();


      // Gaps in datagram sequence number?
      int mycpuNumber      = m_digiEvent->getMetaEvent().datagram().crate(); 
      int myDatagramSecNbr = m_digiEvent->getMetaEvent().datagram().datagrams();  
      if (mycpuNumber==enums::Lsf::Epu0) {
        if ((myDatagramSecNbr != previousDatagramGapsEPU0) && ((myDatagramSecNbr-previousDatagramGapsEPU0)!=1)) {
          m_datagramGapsEPU0++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! There was a gap in the datagram sequence number for EPU0! " << iEvent << "   " << myDatagramSecNbr << "   " << previousDatagramGapsEPU0 << std::endl;  
	}
        previousDatagramGapsEPU0 = myDatagramSecNbr;
      }
      if (mycpuNumber==enums::Lsf::Epu1) {
        if ((myDatagramSecNbr != previousDatagramGapsEPU1) && ((myDatagramSecNbr-previousDatagramGapsEPU1)!=1)) {
          m_datagramGapsEPU1++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! There was a gap in the datagram sequence number for EPU1! " << iEvent << "   " << myDatagramSecNbr << "   " << previousDatagramGapsEPU1 << std::endl;  
	}
        previousDatagramGapsEPU1 = myDatagramSecNbr;
      }
      if (mycpuNumber==enums::Lsf::Epu2) {
        if ((myDatagramSecNbr != previousDatagramGapsEPU2) && ((myDatagramSecNbr-previousDatagramGapsEPU2)!=1)) {
          m_datagramGapsEPU2++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! There was a gap in the datagram sequence number for EPU2! " << iEvent << "   " << myDatagramSecNbr << "   " << previousDatagramGapsEPU2 << std::endl;  
	}
        previousDatagramGapsEPU2 = myDatagramSecNbr;
      }
      if (mycpuNumber==enums::Lsf::Siu0) {
        if ((myDatagramSecNbr != previousDatagramGapsSIU0) && ((myDatagramSecNbr-previousDatagramGapsSIU0)!=1) && myDatagramSecNbr!=0) {
          m_datagramGapsSIU0++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! There was a gap in the datagram sequence number for SIU0! " << iEvent << "   " << myDatagramSecNbr << "   " << previousDatagramGapsSIU0 << std::endl;  
	}
        previousDatagramGapsSIU0 = myDatagramSecNbr;
      }
      if (mycpuNumber==enums::Lsf::Siu1) {
        if ((myDatagramSecNbr != previousDatagramGapsSIU1) && ((myDatagramSecNbr-previousDatagramGapsSIU1)!=1) && myDatagramSecNbr!=0) {
          m_datagramGapsSIU1++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! There was a gap in the datagram sequence number for SIU1! " << iEvent << "   " << myDatagramSecNbr << "   " << previousDatagramGapsSIU1 << std::endl;  
 	}
        previousDatagramGapsSIU1 = myDatagramSecNbr;
     }                




      // Events per datagram: Only for EPUs and SIUs!      
      int cpuNumber = m_digiEvent->getMetaEvent().datagram().crate(); 
      if (cpuNumber>-1 && cpuNumber<5) {
        if (m_digiEvent->getMetaEvent().datagram().datagrams() == idDatagram[cpuNumber]) {
          nbrEventsDatagram[cpuNumber]++;
	} 
        if (m_digiEvent->getMetaEvent().datagram().datagrams() != idDatagram[cpuNumber]) {
          if (cpuNumber==enums::Lsf::Epu0) {
	    m_datagramsEPU0->Fill(nbrEventsDatagram[cpuNumber]);
	  }
          if (cpuNumber==enums::Lsf::Epu1) {
	    m_datagramsEPU1->Fill(nbrEventsDatagram[cpuNumber]);
	  }
          if (cpuNumber==enums::Lsf::Epu2) {
	    m_datagramsEPU2->Fill(nbrEventsDatagram[cpuNumber]);
	  }
          if (cpuNumber==enums::Lsf::Siu0) {
	    m_datagramsSIU0->Fill(nbrEventsDatagram[cpuNumber]);
	  }
          if (cpuNumber==enums::Lsf::Siu1) {
	    m_datagramsSIU1->Fill(nbrEventsDatagram[cpuNumber]);
	  }
	  //
          idDatagram[cpuNumber] = m_digiEvent->getMetaEvent().datagram().datagrams();
          nbrEventsDatagram[cpuNumber] = 0;
	}
	// Last event:
        if (iEvent == (m_nEvent-1)) {
          if (nbrEventsDatagram[0] != 0) {
            m_datagramsEPU0->Fill(nbrEventsDatagram[0]);
	  }
          if (nbrEventsDatagram[1] != 0) {
            m_datagramsEPU1->Fill(nbrEventsDatagram[1]);
          }
          if (nbrEventsDatagram[2] != 0) {
            m_datagramsEPU2->Fill(nbrEventsDatagram[2]);
	  }
          if (nbrEventsDatagram[3] != 0) {
            m_datagramsSIU0->Fill(nbrEventsDatagram[3]);
          }
          if (nbrEventsDatagram[4] != 0) {
            m_datagramsSIU1->Fill(nbrEventsDatagram[4]);
	  }
	}
      }


      // Datagrams:
      // Here I make a list of the datagrams and keep the opening reason for the first datagram - all per EPU/SIU:

      // EPU0:
      if (m_digiEvent->getMetaEvent().datagram().crate() == enums::Lsf::Epu0) {
        m_thisDataGramEpu0 = m_digiEvent->getMetaEvent().datagram().datagrams();

        m_nbrEventsDataGramsEpu0++;
        lastDatagramEventEpu0 = iEvent;
      
	// First event in first datagram for EPU0?
        if (firstDatagramEventEpu0 == -1) {
          listDataGramsEpu0.push_back(m_thisDataGramEpu0);

	  m_firstDataGramEpu0    = m_thisDataGramEpu0;
	  m_beginRunDataGramEpu0 = 0;
	  int firstDataGramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
	  if (firstDataGramOpen == enums::Lsf::Open::Start) {
	    m_beginRunDataGramEpu0 = 1;
          }
          firstDatagramEventEpu0 = 1;
	}

        if (firstDatagramEventEpu0 != -1) {
          // This is not perfect, but I'll remove the presumably small number of duplicates at the end:
          if (m_thisDataGramEpu0 != m_previousDataGramEpu0 && m_thisDataGramEpu0 != m_previousPreviousDataGramEpu0) {
            listDataGramsEpu0.push_back(m_thisDataGramEpu0);
	  }
        }
        // Keep datagram sequence numbers for two previous events. Not perfect, but good enough!
        unsigned int tmpDataGramEpu0   = m_previousDataGramEpu0;
        m_previousDataGramEpu0         = m_thisDataGramEpu0;
        m_previousPreviousDataGramEpu0 = tmpDataGramEpu0;
      }


      // EPU1:
      if (m_digiEvent->getMetaEvent().datagram().crate() == enums::Lsf::Epu1) {
        m_thisDataGramEpu1 = m_digiEvent->getMetaEvent().datagram().datagrams();

        m_nbrEventsDataGramsEpu1++;
        lastDatagramEventEpu1 = iEvent;
      
	// First event in first datagram for EPU1?
        if (firstDatagramEventEpu1 == -1) {
          listDataGramsEpu1.push_back(m_thisDataGramEpu1);

	  m_firstDataGramEpu1    = m_thisDataGramEpu1;
	  m_beginRunDataGramEpu1 = 0;
	  int firstDataGramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
	  if (firstDataGramOpen == enums::Lsf::Open::Start) {
	    m_beginRunDataGramEpu1 = 1;
          }
          firstDatagramEventEpu1 = 1;
	}

        if (firstDatagramEventEpu1 != -1) {
          // This is not perfect, but I'll remove the presumably small number of duplicates at the end:
          if (m_thisDataGramEpu1 != m_previousDataGramEpu1 && m_thisDataGramEpu1 != m_previousPreviousDataGramEpu1) {
            listDataGramsEpu1.push_back(m_thisDataGramEpu1);
	  }
        }
        // Keep datagram sequence numbers for two previous events. Not perfect, but good enough!
        unsigned int tmpDataGramEpu1   = m_previousDataGramEpu1;
        m_previousDataGramEpu1         = m_thisDataGramEpu1;
        m_previousPreviousDataGramEpu1 = tmpDataGramEpu1;
      }


      // EPU2:
      if (m_digiEvent->getMetaEvent().datagram().crate() == enums::Lsf::Epu2) {
        m_thisDataGramEpu2 = m_digiEvent->getMetaEvent().datagram().datagrams();

        m_nbrEventsDataGramsEpu2++;
        lastDatagramEventEpu2 = iEvent;
      
	// First event in first datagram for EPU0?
        if (firstDatagramEventEpu2 == -1) {
          listDataGramsEpu2.push_back(m_thisDataGramEpu2);

	  m_firstDataGramEpu2    = m_thisDataGramEpu2;
	  m_beginRunDataGramEpu2 = 0;
	  int firstDataGramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
	  if (firstDataGramOpen == enums::Lsf::Open::Start) {
	    m_beginRunDataGramEpu2 = 1;
          }
          firstDatagramEventEpu2 = 1;
	}

        if (firstDatagramEventEpu2 != -1) {
          // This is not perfect, but I'll remove the presumably small number of duplicates at the end:
          if (m_thisDataGramEpu2 != m_previousDataGramEpu2 && m_thisDataGramEpu2 != m_previousPreviousDataGramEpu2) {
            listDataGramsEpu2.push_back(m_thisDataGramEpu2);
	  }
        }
        // Keep datagram sequence numbers for two previous events. Not perfect, but good enough!
        unsigned int tmpDataGramEpu2   = m_previousDataGramEpu2;
        m_previousDataGramEpu2         = m_thisDataGramEpu2;
        m_previousPreviousDataGramEpu2 = tmpDataGramEpu2;
      }


      // SIU0:
      if (m_digiEvent->getMetaEvent().datagram().crate() == enums::Lsf::Siu0) {
        m_thisDataGramSiu0 = m_digiEvent->getMetaEvent().datagram().datagrams();

        // Look for gaps and commanded acquisisations:
        if (m_thisDataGramSiu0 < m_previousDataGramSiu0) {
          m_counterCyclesSiu0++;
          if (m_thisDataGramSiu0 != 0) {
            m_counterDataDiagramsSiu0 = m_counterDataDiagramsSiu0 + m_thisDataGramSiu0;
	  }
	} 
        if ((m_thisDataGramSiu0>m_previousDataGramSiu0) && (m_thisDataGramSiu0-m_previousDataGramSiu0) > 1) {
  	  m_counterDataDiagramsSiu0 = m_counterDataDiagramsSiu0 + (m_thisDataGramSiu0-m_previousDataGramSiu0-1);
	}
      

        // This is not very useful for LCI runs, but I'll keep it for now:
        m_nbrEventsDataGramsSiu0++;
        lastDatagramEventSiu0 = iEvent;
      
	// First event in first datagram for SIU0?
        if (firstDatagramEventSiu0 == -1) {
          listDataGramsSiu0.push_back(m_thisDataGramSiu0);

	  m_firstDataGramSiu0    = m_thisDataGramSiu0;
	  m_beginRunDataGramSiu0 = 0;
	  int firstDataGramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
	  if (firstDataGramOpen == enums::Lsf::Open::Start) {
	    m_beginRunDataGramSiu0 = 1;
          }
          firstDatagramEventSiu0 = 1;
	}

        if (firstDatagramEventSiu0 != -1) {
          // This is not perfect, but I'll remove the presumably small number of duplicates at the end:
          if (m_thisDataGramSiu0 != m_previousDataGramSiu0 && m_thisDataGramSiu0 != m_previousPreviousDataGramSiu0) {
            listDataGramsSiu0.push_back(m_thisDataGramSiu0);
	  }
        }
        // Keep datagram sequence numbers for two previous events. Not perfect, but good enough!
        unsigned int tmpDataGramSiu0   = m_previousDataGramSiu0;
        m_previousDataGramSiu0         = m_thisDataGramSiu0;
        m_previousPreviousDataGramSiu0 = tmpDataGramSiu0;
      }


      // SIU1:
      if (m_digiEvent->getMetaEvent().datagram().crate() == enums::Lsf::Siu1) {
        m_thisDataGramSiu1 = m_digiEvent->getMetaEvent().datagram().datagrams();

        // Look for gaps and commanded acquisisations:
        if (m_thisDataGramSiu1 < m_previousDataGramSiu1) {
          m_counterCyclesSiu1++;
          if (m_thisDataGramSiu1 != 0) {
            m_counterDataDiagramsSiu1 = m_counterDataDiagramsSiu1 + m_thisDataGramSiu1;
	  }
	} 
        if ((m_thisDataGramSiu1>m_previousDataGramSiu1) && (m_thisDataGramSiu1-m_previousDataGramSiu1) > 1) {
  	  m_counterDataDiagramsSiu1 = m_counterDataDiagramsSiu1 + (m_thisDataGramSiu1-m_previousDataGramSiu1-1);
	}

        // This is not very useful for LCI runs, but I'll keep it for now:
        m_nbrEventsDataGramsSiu1++;
        lastDatagramEventSiu1 = iEvent;
      
	// First event in first datagram for SIU1?
        if (firstDatagramEventSiu1 == -1) {
          listDataGramsSiu1.push_back(m_thisDataGramSiu1);

	  m_firstDataGramSiu1    = m_thisDataGramSiu1;
	  m_beginRunDataGramSiu1 = 0;
	  int firstDataGramOpen = m_digiEvent->getMetaEvent().datagram().openAction();
	  if (firstDataGramOpen == enums::Lsf::Open::Start) {
	    m_beginRunDataGramSiu1 = 1;
          }
          firstDatagramEventSiu1 = 1;
	}

        if (firstDatagramEventSiu1 != -1) {
          // This is not perfect, but I'll remove the presumably small number of duplicates at the end:
          if (m_thisDataGramSiu1 != m_previousDataGramSiu1 && m_thisDataGramSiu1 != m_previousPreviousDataGramSiu1) {
            listDataGramsSiu1.push_back(m_thisDataGramSiu1);
	  }
        }
        // Keep datagram sequence numbers for two previous events. Not perfect, but good enough!
        unsigned int tmpDataGramSiu1   = m_previousDataGramSiu1;
        m_previousDataGramSiu1         = m_thisDataGramSiu1;
        m_previousPreviousDataGramSiu1 = tmpDataGramSiu1;
      }



      // For the last event, we know we have all the datagrams for all the EPU/SIUs:
      if (iEvent == m_nEvent-1) {

        // We only have two EPUs or one SIU in each run:

	// EPU0:
        if (firstDatagramEventEpu0 == 1) {

          // Get rid of duplicates:
          listDataGramsEpu0.sort();
          listDataGramsEpu0.unique();

    	  // Keep total number of datagrams in the run: 
          m_nbrDataGramsEpu0 = listDataGramsEpu0.size();

	  std::list<int>::iterator p;
          for (p = listDataGramsEpu0.begin(); p != listDataGramsEpu0.end(); p++) {       
            if (p != listDataGramsEpu0.begin()) {
              int diff = *p - *(--p);
              p++;
              if (diff > 1) {
                m_counterDataDiagramsEpu0 = m_counterDataDiagramsEpu0 + diff - 1;
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for EPU0!" << std::endl;
	      }
	    }
 	  } 
	  //std::cout << "Datagrams for EPU0 are " << std::endl;
	  //std::copy(listDataGramsEpu0.begin(),listDataGramsEpu0.end(),std::ostream_iterator<int>(std::cout,"\n"));
        }

	// EPU1:
        if (firstDatagramEventEpu1 == 1) {

          // Get rid of duplicates:
          listDataGramsEpu1.sort();
          listDataGramsEpu1.unique();

    	  // Keep total number of datagrams in the run: 
          m_nbrDataGramsEpu1 = listDataGramsEpu1.size();

	  std::list<int>::iterator p;
          for (p = listDataGramsEpu1.begin(); p != listDataGramsEpu1.end(); p++) {       
            if (p != listDataGramsEpu1.begin()) {
              int diff = *p - *(--p);
              p++;
              if (diff > 1) {
                m_counterDataDiagramsEpu1 = m_counterDataDiagramsEpu1 + diff - 1;
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for EPU1!" << std::endl;
	      }
	    }
 	  } 
	  //std::cout << "Datagrams for EPU1 are " << std::endl;
	  //std::copy(listDataGramsEpu1.begin(),listDataGramsEpu1.end(),std::ostream_iterator<int>(std::cout,"\n"));
        }

	// EPU2:
        if (firstDatagramEventEpu2 == 1) {

          // Get rid of duplicates:
          listDataGramsEpu2.sort();
          listDataGramsEpu2.unique();

    	  // Keep total number of datagrams in the run: 
          m_nbrDataGramsEpu2 = listDataGramsEpu2.size();

	  std::list<int>::iterator p;
          for (p = listDataGramsEpu2.begin(); p != listDataGramsEpu2.end(); p++) {       
            if (p != listDataGramsEpu2.begin()) {
              int diff = *p - *(--p);
              p++;
              if (diff > 1) {
                m_counterDataDiagramsEpu2 = m_counterDataDiagramsEpu2 + diff - 1;
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for EPU2!" << std::endl;
	      }
	    }
 	  } 
	  //std::cout << "Datagrams for EPU2 are " << std::endl;
	  //std::copy(listDataGramsEpu2.begin(),listDataGramsEpu2.end(),std::ostream_iterator<int>(std::cout,"\n"));
        }

	// SIU0:
        if (firstDatagramEventSiu0 == 1) {

          // Get rid of duplicates:
          listDataGramsSiu0.sort();
          listDataGramsSiu0.unique();

    	  // Keep total number of datagrams in the run: 
          m_nbrDataGramsSiu0 = listDataGramsSiu0.size();

	  std::list<int>::iterator p;
          for (p = listDataGramsSiu0.begin(); p != listDataGramsSiu0.end(); p++) {       
            if (p != listDataGramsSiu0.begin()) {
              int diff = *p - *(--p);
              p++;
              if (diff > 1) {
                m_counterDataDiagramsSiu0 = m_counterDataDiagramsSiu0 + diff - 1;
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for SIU0!" << std::endl;
	      }
	    }
 	  } 
	  //std::cout << "Datagrams for SIU0 are " << std::endl;
	  //std::copy(listDataGramsSiu0.begin(),listDataGramsSiu0.end(),std::ostream_iterator<int>(std::cout,"\n"));
        }

	// SIU1:
        if (firstDatagramEventSiu1 == 1) {

          // Get rid of duplicates:
          listDataGramsSiu1.sort();
          listDataGramsSiu1.unique();

    	  // Keep total number of datagrams in the run: 
          m_nbrDataGramsSiu1 = listDataGramsSiu0.size();

	  std::list<int>::iterator p;
          for (p = listDataGramsSiu1.begin(); p != listDataGramsSiu1.end(); p++) {       
            if (p != listDataGramsSiu1.begin()) {
              int diff = *p - *(--p);
              p++;
              if (diff > 1) {
                m_counterDataDiagramsSiu1 = m_counterDataDiagramsSiu1 + diff - 1;
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! We dropped " << (diff - 1) << " datagram(s) before datagram " << (*p) << " for SIU1!" << std::endl;
	      }
	    }
 	  } 
	  //std::cout << "Datagrams for SIU1 are " << std::endl;
	  //std::copy(listDataGramsSiu1.begin(),listDataGramsSiu1.end(),std::ostream_iterator<int>(std::cout,"\n"));
        }
      }
    
     






        

      //if (iEvent > 0) { 
      //  int deltaDataGram = m_thisDataGram - m_previousDataGram;
      //  if (deltaDataGram > 1) {
      //    m_counterDataDiagrams++;  
      //    std::cout << "Warning! We dropped a datagram here!. Current datagram is " << m_thisDataGram << " while the previous datagram was " << m_previousDataGram << ". This happened in event " 
      //              << iEvent << std::endl;
      //  }
      //}
      //m_previousDataGram = m_thisDataGram;


      // Ground ID:
      unsigned int thisGroundID = m_digiEvent->getMetaEvent().run().id();  
      if (iEvent > 0) {
        if (thisGroundID != m_previousGroundID) {
          m_counterGroundID++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The Ground ID changed from " << m_previousGroundID << " to " << thisGroundID << " when going from event " << (iEvent-1) << " to " << iEvent << std::endl;
	}
      }
      m_previousGroundID = thisGroundID;


      // Time tones ging backwards?
      UInt_t thisTimeToneCurrentSec  = m_digiEvent->getMetaEvent().time().current().timeSecs();
      UInt_t thisTimeTonePreviousSec = m_digiEvent->getMetaEvent().time().previous().timeSecs();

      UInt_t thisTimeToneCurrentTicks  = m_digiEvent->getMetaEvent().time().current().timeHack().ticks();
      UInt_t thisTimeTonePreviousTicks = m_digiEvent->getMetaEvent().time().previous().timeHack().ticks();
   
      if ( (thisTimeToneCurrentSec < previousTimeToneCurrentSec) || (thisTimeTonePreviousSec < previousTimeTonePreviousSec)) {
        m_backwardsTimeTone++;
	std::cout << "   " << std::endl; 
	std::cout << "Warning! Time tones seem to go backwards in event " << iEvent << " with GEM ID " << m_digiEvent->getMetaEvent().scalers().sequence() << std::endl;
	std::cout << "Current and previous time tones seconds are: " << thisTimeToneCurrentSec << "   " << previousTimeToneCurrentSec << "   " 
                  << thisTimeTonePreviousSec << "   " << previousTimeTonePreviousSec << std::endl;
	std::cout << "Current and previous time tones ticks are: " << thisTimeToneCurrentTicks << "   " << previousTimeToneCurrentTicks << "   " 
                  << thisTimeTonePreviousTicks << "   " << previousTimeTonePreviousTicks << std::endl;
      }
      previousTimeToneCurrentSec    = thisTimeToneCurrentSec;
      previousTimeTonePreviousSec   = thisTimeTonePreviousSec;
      previousTimeToneCurrentTicks  = thisTimeToneCurrentTicks;
      previousTimeTonePreviousTicks = thisTimeTonePreviousTicks;


      // Current and previous time tones identical?
      if (thisTimeToneCurrentSec==thisTimeTonePreviousSec && thisTimeToneCurrentTicks==thisTimeTonePreviousTicks) {
        m_identicalTimeTones++;
	std::cout << "   " << std::endl; 
	std::cout << "Warning! Current and previous time tones are identical in event " << iEvent << " with GEM ID " << m_digiEvent->getMetaEvent().scalers().sequence() << std::endl;

	std::cout << "Current and previous time tones are both: " << thisTimeToneCurrentSec << "   " << thisTimeToneCurrentTicks << std::endl;
      }   

 

      // Time tone counters and flags:
      if (m_digiEvent->getMetaEvent().time().current().incomplete() != 0) {
        m_nbrIncomplete++;
      }
      if (m_digiEvent->getMetaEvent().time().current().missingGps() != 0) {
        m_nbrMissingGps++;
      }
      if (m_digiEvent->getMetaEvent().time().current().missingCpuPps() != 0) {
        m_nbrMissingCpuPps++;
      }
      if (m_digiEvent->getMetaEvent().time().current().missingLatPps() != 0) {
        m_nbrMissingLatPps++;
      }
      if (m_digiEvent->getMetaEvent().time().current().missingTimeTone() != 0) {
        m_nbrMissingTimeTone++;
      }

      // Trigger/deadzone rates per time interval:
      eventCounter++;

      ULong64_t thisDeadZone  = m_digiEvent->getMetaEvent().scalers().deadzone();
      ULong64_t thisDiscarded = m_digiEvent->getMetaEvent().scalers().discarded();

      ULong64_t thisElapsed   = m_digiEvent->getMetaEvent().scalers().elapsed();
      ULong64_t thisLiveTime  = m_digiEvent->getMetaEvent().scalers().livetime();
      ULong64_t thisPrescaled = m_digiEvent->getMetaEvent().scalers().prescaled();
      ULong64_t thisSequence  = m_digiEvent->getMetaEvent().scalers().sequence();

      // For livetime calculations:
      if (livetimeFirst == 0) {
        livetimeFirst = thisLiveTime;
      }       


      if (iEvent > 0) { 
        Long64_t deltaDeadZone = thisDeadZone - previousDeadZone; 
        deadzoneCounter         = deadzoneCounter + deltaDeadZone;        

        if (deltaDeadZone < 0) { 
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended DeadZone counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousDeadZone << " to " << thisDeadZone << " i.e. a change of " << deltaDeadZone << " ticks!" << std::endl;
	}

        Long64_t deltaDiscarded = thisDiscarded - previousDiscarded; 
        discardedCounter         = discardedCounter + deltaDiscarded;
        if (deltaDiscarded < 0) { 
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended Discarded counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousDiscarded << " to " << thisDiscarded << " i.e. a change of " << deltaDiscarded << " ticks!" << std::endl;
	}

	Long64_t deltaElapsed = thisElapsed - previousElapsed;
        m_timeIntervalElapsed->Fill(0.00005*deltaElapsed);
        if (deltaElapsed < 0) {
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended Elapsed counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousElapsed << " to " << thisElapsed << " i.e. a change of " << deltaElapsed << " ticks!" << std::endl;
	}

	Long64_t deltaLiveTime = thisLiveTime - previousLiveTime;
        if (deltaLiveTime < 0) {
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended Livetime counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousLiveTime << " to " << thisLiveTime << " i.e. a change of " << deltaLiveTime << " ticks!" << std::endl;
	}

	Long64_t deltaPrescaled = thisPrescaled - previousPrescaled;
        if (deltaPrescaled < 0) {
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended Prescaled counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousPrescaled << " to " << thisPrescaled << " i.e. a change of " << deltaPrescaled << " ticks!" << std::endl;
	}

	Long64_t deltaSequence = thisSequence - previousSequence;
        if (deltaSequence < 0) {
          m_extendedCountersFlag++;
  	  std::cout << "   " << std::endl; 
	  std::cout << "Warning! The extended Sequence counter DECREASED from event " << (iEvent-1) << " to event " << iEvent << std::endl;
	  std::cout << "         It went from " << previousSequence << " to " << thisSequence << " i.e. a change of " << deltaSequence << " ticks!" << std::endl;
	}
      }  
      previousDeadZone  = thisDeadZone;
      previousDiscarded = thisDiscarded;
      previousElapsed   = thisElapsed;
      previousSequence  = thisSequence;
      previousPrescaled = thisPrescaled;
      previousLiveTime  = thisLiveTime;


      ULong64_t thisElapsedTime = m_digiEvent->getMetaEvent().scalers().elapsed();

      Long64_t deltaTime = thisElapsedTime - elapsedTimeFirst; 
      Long64_t cutTime   = intervalCounter*deltaTimeInterval;  

      // Trigger rates:
      if (deltaTime >= cutTime) {
        float rate;
        float rateLivetimeCorrected;
        float liveTimeFraction;
        if (deltaTimeInterval != 0) {
          rate = (float) eventCounter/ (float) (deltaTimeInterval * 50 * std::pow(10.0, -9));

          // Livetime:
          livetimeLast          = thisLiveTime;
          liveTimeFraction      = (livetimeLast - livetimeFirst) / (float) (deltaTimeInterval);
          
          if (liveTimeFraction <= 0) {
  	    std::cout << "   " << std::endl; 
	    std::cout << "Problem! The livetime is coming out as " << liveTimeFraction << "   " << livetimeLast << "   " << livetimeFirst << "   " << deltaTimeInterval << "   " << " in event "  
                      << iEvent << std::endl;
            rateLivetimeCorrected = -1.0;
	  } else { 
            rateLivetimeCorrected = rate / liveTimeFraction;
	  }
          liveTimeFraction = liveTimeFraction * 100.0;
	} else {
	  rate                  = -1.0;
          rateLivetimeCorrected = -1.0;
	  liveTimeFraction      = -1.0;
	}
        m_triggerRate->Fill(intervalCounter-1,rate);
        m_triggerLivetimeRate->Fill(intervalCounter-1,rateLivetimeCorrected);
        m_livetimeRate->Fill(intervalCounter-1,liveTimeFraction);

	// Deadzone rates:
        if (deltaTimeInterval != 0) {
          rate = (double) deadzoneCounter/ (double) (deltaTimeInterval * 50 * std::pow(10.0, -9));
        } else {
          rate = -1.0;
        }
        m_deadzoneRate->Fill(intervalCounter-1,rate);

	// Discarded rates:
        if (deltaTimeInterval != 0) {
          rate = (float) discardedCounter/ (float) (deltaTimeInterval * 50 * std::pow(10.0, -9));
        } else {
          rate = -1.0;
        }
        m_discardedRate->Fill(intervalCounter-1,rate);

	// Reset counters:
        intervalCounter++;

        eventCounter     = 0;
        deadzoneCounter  = 0;
        discardedCounter = 0;

        livetimeFirst = 0;
        livetimeLast  = 0;
      }


      // EPU:
      int crate = m_digiEvent->getMetaEvent().datagram().crate();
      m_epu->Fill(crate);


      // GEM discarded and deadzone events:
      ULong64_t thisGemDiscarded = m_digiEvent->getMetaEvent().scalers().discarded();
      ULong64_t thisGemDeadzone  = m_digiEvent->getMetaEvent().scalers().deadzone();

      int thisGemDeltaEventTime       = m_digiEvent->getGem().getDeltaEventTime();
      int thisGemWDeltaWindowOpenTime = m_digiEvent->getGem().getDeltaWindowOpenTime();

      if (iEvent > 0) {
        // Gem discarded delta wrt the previous event:
        Long64_t delta = thisGemDiscarded - previousGemDiscarded;
        m_gemDiscarded->Fill(delta);

        // Gem deadzone delta wrt the previous event:
        Long64_t deltaDeadzone = thisGemDeadzone - previousGemDeadzone;
        m_gemDeadzone->Fill(deltaDeadzone);


        // Fill time histo for non-saturated events:
        if (thisGemWDeltaWindowOpenTime<65500 && thisGemDeltaEventTime<65500) {

          // Get time quantity we want to look at:
          int deltaTime = thisGemDeltaEventTime - thisGemWDeltaWindowOpenTime;

          // Fill histogram:
          m_gemDiscardedTime->Fill(deltaTime);
        }
      }
      previousGemDiscarded = thisGemDiscarded;
      previousGemDeadzone  = thisGemDeadzone;


      // Only for FSW:
      if (m_isLATTE != 1) {
        
        //
        // Time from Mission elapsed time to Unix time:
        //
        Int_t deltaTimeUgly = 978307200; 

        double myTimeStamp;

        // LAT nominal system clock:
	double LATSystemClock = 20000000.0;

	// Rollover offset of 25 bit GEM counter:
        double RollOver = 33554432.0;

	// Number of ticks between current event and the current 1-PPS:
	double tmpTicks1 = double (m_digiEvent->getMetaEvent().time().timeTicks()) - double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks());

	// Rollover? Should never be more than one! 
	if (tmpTicks1 < 0) {
	  tmpTicks1 = tmpTicks1 + RollOver;
	}

	// Check that the two TimeTones are OK:
	if (!(m_digiEvent->getMetaEvent().time().current().incomplete()) &&
            !(m_digiEvent->getMetaEvent().time().current().flywheeling()) &&
	    !(m_digiEvent->getMetaEvent().time().current().missingCpuPps()) &&
	    !(m_digiEvent->getMetaEvent().time().current().missingLatPps()) &&
	    !(m_digiEvent->getMetaEvent().time().current().missingTimeTone()) &&
	    !(m_digiEvent->getMetaEvent().time().previous().incomplete()) &&
	    !(m_digiEvent->getMetaEvent().time().previous().flywheeling()) &&
	    !(m_digiEvent->getMetaEvent().time().previous().missingCpuPps()) &&
	    !(m_digiEvent->getMetaEvent().time().previous().missingLatPps()) &&
	    !(m_digiEvent->getMetaEvent().time().previous().missingTimeTone()) &&
	    // Just in case (to protect from 1/0):
            ((m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) != (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks())) &&
            // If more than one second, must use nominal LAT clock value:
	    ( (m_digiEvent->getMetaEvent().time().current().timeSecs() - m_digiEvent->getMetaEvent().time().previous().timeSecs()) == 1)) {

	  // Then use full formula for correcting system clock drift using last two TimeTones i.e. extrapolation
	  double tmpTicks2 = double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) - double (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks());

	  // Rollover? Should never be more than one rollover! 
	  if (tmpTicks2 < 0) {
	    tmpTicks2 = tmpTicks2 + RollOver;
	  }

	  // Timestamp:
	  myTimeStamp = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/tmpTicks2);
	  myTimeStamp = myTimeStamp + deltaTimeUgly;

	} else {

	  // Cannot use TimeTone(s) - will assume nominal value for the LAT system clock                                                                                              
   	  myTimeStamp = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/LATSystemClock);
	  myTimeStamp = myTimeStamp + deltaTimeUgly;
	}
        double myTimeDiff = m_digiEvent->getCcsds().getUtc() - myTimeStamp;
        if (myTimeDiff < 0) {
	  std::cout.setf(ios::fixed);
	  std::cout << "Problem! Time difference is negative in event " << iEvent << "  " << myTimeDiff << "   " << std::setprecision(20) << (m_digiEvent->getCcsds().getUtc()) 
                    << "   " << myTimeStamp << std::setprecision(5) << std::endl;
	}

	int mycpuNumber=  m_digiEvent->getMetaEvent().datagram().crate();
	if (mycpuNumber>-1 && mycpuNumber<5) {
	  if (cpuNumber==enums::Lsf::Epu0) {
	    m_deltaTimeDGCTEvtEPU0->Fill(myTimeDiff);
          }
	  if (mycpuNumber==enums::Lsf::Epu1) {
	    m_deltaTimeDGCTEvtEPU1->Fill(myTimeDiff);
          }
	  if (mycpuNumber==enums::Lsf::Epu2) {
	    m_deltaTimeDGCTEvtEPU2->Fill(myTimeDiff);
          }
	  if (mycpuNumber==enums::Lsf::Siu0) {
	    m_deltaTimeDGCTEvtSIU0->Fill(myTimeDiff);
          }
	  if (mycpuNumber==enums::Lsf::Siu1) {
	    m_deltaTimeDGCTEvtSIU1->Fill(myTimeDiff);
          }
	}

        // Delta Event ID:
	if (mycpuNumber>-1 && mycpuNumber<5) {

	  if (mycpuNumber==enums::Lsf::Epu0) {
            currentEventIDEPU0 = m_digiEvent->getMetaEvent().scalers().sequence();
            if (previousEventIDEPU0 != 0) {	
              if (currentEventIDEPU0 <= previousEventIDEPU0) {
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! Problems with the delta Event ID in EPU0! " << iEvent << "   " << currentEventIDEPU0 << "   " << previousEventIDEPU0 << std::endl;
	      }
              double DeltaEventID = double (currentEventIDEPU0) - double (previousEventIDEPU0);
              m_deltaEventIDEPU0->Fill(DeltaEventID);
	    }
            previousEventIDEPU0 = currentEventIDEPU0;
	  }

	  if (mycpuNumber==enums::Lsf::Epu1) {
            currentEventIDEPU1 = m_digiEvent->getMetaEvent().scalers().sequence();
            if (previousEventIDEPU1 != 0) {	
              if (currentEventIDEPU1 <= previousEventIDEPU1) {
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! Problems with the delta Event ID in EPU1! " << iEvent << "   " << currentEventIDEPU1 << "   " << previousEventIDEPU1 << std::endl;
	      }
              double DeltaEventID = double (currentEventIDEPU1) - double (previousEventIDEPU1);
              m_deltaEventIDEPU1->Fill(DeltaEventID);
	    }
            previousEventIDEPU1 = currentEventIDEPU1;
	  }

	  if (mycpuNumber==enums::Lsf::Epu2) {
            currentEventIDEPU2 = m_digiEvent->getMetaEvent().scalers().sequence();
            if (previousEventIDEPU2 != 0) {	
              if (currentEventIDEPU2 <= previousEventIDEPU2) {
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! Problems with the delta Event ID in EPU2! " << iEvent << "   " << currentEventIDEPU2 << "   " << previousEventIDEPU2 << std::endl;
	      } 
              double DeltaEventID = double (currentEventIDEPU2) - double (previousEventIDEPU2);
              m_deltaEventIDEPU2->Fill(DeltaEventID);
	    }
            previousEventIDEPU2 = currentEventIDEPU2;
	  }

	  if (mycpuNumber==enums::Lsf::Siu0) {
            currentEventIDSIU0 = m_digiEvent->getMetaEvent().scalers().sequence();
            if (previousEventIDSIU0 != 0) {	
              if (currentEventIDSIU0 <= previousEventIDSIU0) {
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! Problems with the delta Event ID in SIU0! " << iEvent << "   " << currentEventIDSIU0 << "   " << previousEventIDSIU0 << std::endl;
	      } 
              double DeltaEventID = double (currentEventIDSIU0) - double (previousEventIDSIU0);
              m_deltaEventIDSIU0->Fill(DeltaEventID);
	    }
            previousEventIDSIU0 = currentEventIDSIU0;
	  }

	  if (mycpuNumber==enums::Lsf::Siu1) {
            currentEventIDSIU1 = m_digiEvent->getMetaEvent().scalers().sequence();
            if (previousEventIDSIU1 != 0) {	
              if (currentEventIDSIU1 <= previousEventIDSIU1) {
  	        std::cout << "   " << std::endl; 
	        std::cout << "Warning! Problems with the delta Event ID in SIU1! " << iEvent << "   " << currentEventIDSIU1 << "   " << previousEventIDSIU1 << std::endl;
	      } 
              double DeltaEventID = double (currentEventIDSIU1) - double (previousEventIDSIU1);
              m_deltaEventIDSIU1->Fill(DeltaEventID);
	    }
            previousEventIDSIU1 = currentEventIDSIU1;
	  }
	}
      }


      // PPC time:
      UInt_t uPpcT = m_digiEvent->getEbfUpperPpcTimeBase();
      UInt_t lPpcT = m_digiEvent->getEbfLowerPpcTimeBase();
      static UInt_t prevUPpcT = uPpcT;
      static UInt_t prevLPpcT = lPpcT;

      //
      // Time from Mission elapsed time to Unix time:
      //
      Int_t deltaTimeUgly = 978307200;

      if(iEvent == 0) {
        if (m_isLATTE == 1) {
	  m_startTime = m_digiEvent->getEbfTimeSec();
        } else {

	  // LAT nominal system clock:
	  double LATSystemClock = 20000000.0;

	  // Rollover offset of 25 bit GEM counter:
          double RollOver = 33554432.0;

          // Number of ticks between current event and the current 1-PPS:
	  double tmpTicks1 = double (m_digiEvent->getMetaEvent().time().timeTicks()) - double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks());

	  // Rollover? Should never be more than one! 
	  if (tmpTicks1 < 0) {
	    tmpTicks1 = tmpTicks1 + RollOver;
	  }
          	  
	  // Check that the two TimeTones are OK:
	  if (!(m_digiEvent->getMetaEvent().time().current().incomplete()) &&
              !(m_digiEvent->getMetaEvent().time().current().flywheeling()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingCpuPps()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingLatPps()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingTimeTone()) &&
              !(m_digiEvent->getMetaEvent().time().previous().incomplete()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().flywheeling()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingCpuPps()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingLatPps()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingTimeTone()) &&
              // Just in case (to protect from 1/0):
              ((m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) != (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks())) &&
              // If more than one second, must use nominal value of LAT clock:
	      ( (m_digiEvent->getMetaEvent().time().current().timeSecs() - m_digiEvent->getMetaEvent().time().previous().timeSecs()) == 1)) { 

 	    // Then use full formula for correcting system clock drift using last two TimeTones i.e. extrapolation
	    double tmpTicks2 = double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) - double (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks());

	    // Rollover? Should never be more than one rollover! 
	    if (tmpTicks2 < 0) {
 	      tmpTicks2 = tmpTicks2 + RollOver;
	    }

	    // Timestamp: 
	    m_startTime = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/tmpTicks2);
            m_startTime = m_startTime + deltaTimeUgly;

	  } else {
	  	  
	    // Cannot use TimeTone(s) - will assume nominal value for the LAT system clock
            m_startTime = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/LATSystemClock);
            m_startTime = m_startTime + deltaTimeUgly;
	  }
	}
      }
      else {
	// convert 16 MHZ clock to ms
	assert(uPpcT >= prevUPpcT);

	double interval;

        // note a long can only hold 32 bit
	static const long temp = 256*256*256*255;

	if(lPpcT < prevLPpcT) { // roll over
	  interval = double(uPpcT - prevUPpcT - 1) * 268435.456 + 
	  double(temp - prevLPpcT + lPpcT + 256*256*256) / 16000.;
	}
	else {
	  interval = double(uPpcT - prevUPpcT) * 268435.456 + 
	    double_t(lPpcT - prevLPpcT) / 16000.;
	}

	//if ( interval < 500. ) {
	  m_timeInterval->Fill(interval);
	  //}
        if ( interval < 1. ) {
 	  m_timeIntervalCut->Fill(interval);
	}
	prevUPpcT = uPpcT;
	prevLPpcT = lPpcT;
      }

      if(iEvent == m_nEvent-1) {
        if (m_isLATTE == 1) { 
	  m_endTime = m_digiEvent->getEbfTimeSec();

	} else {

	  // LAT nominal system clock:
	  double LATSystemClock = 20000000.0;

	  // Rollover offset of 25 bit GEM counter:
          double RollOver = 33554432.0;

          // Number of ticks between current event and the current 1-PPS:
	  double tmpTicks1 = double (m_digiEvent->getMetaEvent().time().timeTicks()) - double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks());

	  // Rollover? Should never be more than one! 
	  if (tmpTicks1 < 0) {
	    tmpTicks1 = tmpTicks1 + RollOver;
	  }
          
	  // Check that the two TimeTones are OK and different:
	  if (!(m_digiEvent->getMetaEvent().time().current().incomplete()) &&
              !(m_digiEvent->getMetaEvent().time().current().flywheeling()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingCpuPps()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingLatPps()) &&
	      !(m_digiEvent->getMetaEvent().time().current().missingTimeTone()) &&
              !(m_digiEvent->getMetaEvent().time().previous().incomplete()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().flywheeling()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingCpuPps()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingLatPps()) &&
	      !(m_digiEvent->getMetaEvent().time().previous().missingTimeTone()) &&
              // Just in case (to protect from 1/0):
              ((m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) != (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks())) &&
	      // If more than one second, must use nominal value:
	      ( (m_digiEvent->getMetaEvent().time().current().timeSecs() - m_digiEvent->getMetaEvent().time().previous().timeSecs()) == 1)) { 

 	    // Then use full formula for correcting system clock drift using last two TimeTones i.e. extrapolation
	    double tmpTicks2 = double (m_digiEvent->getMetaEvent().time().current().timeHack().ticks()) - double (m_digiEvent->getMetaEvent().time().previous().timeHack().ticks());

	    // Rollover? Should never be more than one rollover! 
	    if (tmpTicks2 < 0) {
	      tmpTicks2 = tmpTicks2 + RollOver;
	    }


	    // Timestamp: 
	    m_endTime = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/tmpTicks2);
            m_endTime = m_endTime + deltaTimeUgly;

	  } else {

	    // Cannot use TimeTone(s) - will assume nominal value for the LAT system clock:
	    m_endTime = double (m_digiEvent->getMetaEvent().time().current().timeSecs()) + (tmpTicks1/LATSystemClock);
            m_endTime = m_endTime + deltaTimeUgly;
	  }
	}
      }
    }

    // Analyze Recon file:
    if(m_reconFile) {
      m_reconBranch->GetEntry(iEvent);
      analyzeReconTree();
    }

  }

  // Last event for the last datagram per EPU/SIU:
  // EPU0:
  if (lastDatagramEventEpu0 != -1) {
    m_digiBranch->GetEntry(lastDatagramEventEpu0);
    if (m_digiEvent->getMetaEvent().datagram().crate() != enums::Lsf::Epu0) {
      std::cout << "   " << std::endl; 
      std::cout << "Anders! You fucked up! This should have been been from EPU0, but it's not! It is " << m_digiEvent->getMetaEvent().datagram().crate() << std::endl;
    } else {
      m_endRunDataGramEpu0 = 0;
      m_fullDataGramEpu0   = 0;
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      if (lastActionDataGram == enums::Lsf::Close::Stop) {
        m_endRunDataGramEpu0 = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_fullDataGramEpu0 = 1;
      }
      if (m_endRunDataGramEpu0==0 && m_fullDataGramEpu0==0) {
  	std::cout << "   " << std::endl; 
        std::cout << "Warning! The last datagram for EPU0 was not closed because we reached end of run or because it was full! The datagram closing reason was " << lastReasonDataGram
	  	  << " and the datagram closing action was " << lastActionDataGram << std::endl;
      }
    }
    m_digiEvent->Clear();
  }

  if (lastDatagramEventEpu1 != -1) {
    m_digiBranch->GetEntry(lastDatagramEventEpu1);
    if (m_digiEvent->getMetaEvent().datagram().crate() != enums::Lsf::Epu1) {
      std::cout << "   " << std::endl; 
      std::cout << "Anders! You fucked up! This should have been been from EPU1, but it's not! It is " << m_digiEvent->getMetaEvent().datagram().crate() << std::endl;
    } else {
      m_endRunDataGramEpu1 = 0;
      m_fullDataGramEpu1   = 0;
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      if (lastActionDataGram == enums::Lsf::Close::Stop) {
        m_endRunDataGramEpu1 = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_fullDataGramEpu1 = 1;
      }
      if (m_endRunDataGramEpu1==0 && m_fullDataGramEpu1==0) {
  	std::cout << "   " << std::endl; 
        std::cout << "Warning! The last datagram for EPU1 was not closed because we reached end of run or because it was full! The datagram closing reason was " << lastReasonDataGram
	  	  << " and the datagram closing action was " << lastActionDataGram << std::endl;
      }
    }
   m_digiEvent->Clear();
  }

  if (lastDatagramEventEpu2 != -1) {
    m_digiBranch->GetEntry(lastDatagramEventEpu2);
    if (m_digiEvent->getMetaEvent().datagram().crate() != enums::Lsf::Epu2) {
      std::cout << "   " << std::endl; 
      std::cout << "Anders! You fucked up! This should have been been from EPU2, but it's not! It is " << m_digiEvent->getMetaEvent().datagram().crate() << std::endl;
    } else {
      m_endRunDataGramEpu2 = 0;
      m_fullDataGramEpu2   = 0;
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      if (lastActionDataGram == enums::Lsf::Close::Stop) {
        m_endRunDataGramEpu2 = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_fullDataGramEpu2 = 1;
      }
      if (m_endRunDataGramEpu2==0 && m_fullDataGramEpu2==0) {
  	std::cout << "   " << std::endl; 
        std::cout << "Warning! The last datagram for EPU2 was not closed because we reached end of run or because it was full! The datagram closing reason was " << lastReasonDataGram
	  	  << " and the datagram closing action was " << lastActionDataGram << std::endl;
      }
    }
    m_digiEvent->Clear();
  }


  if (lastDatagramEventSiu0 != -1) {
    m_digiBranch->GetEntry(lastDatagramEventSiu0);
    if (m_digiEvent->getMetaEvent().datagram().crate() != enums::Lsf::Siu0) {
      std::cout << "   " << std::endl; 
      std::cout << "Anders! You fucked up! This should have been been from SIU0, but it's not! It is " << m_digiEvent->getMetaEvent().datagram().crate() << std::endl;
    } else {
      m_endCountDataGramSiu0 = 0;
      m_fullDataGramSiu0     = 0;
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      if (lastReasonDataGram == enums::Lsf::Close::CountedOut) {
        m_endCountDataGramSiu0 = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_fullDataGramSiu0 = 1;
      }
      if (m_fullDataGramSiu0==0 && m_endCountDataGramSiu0==0) {
  	std::cout << "   " << std::endl; 
        std::cout << "Warning! The last datagram for SIU0 was not closed because it was full or because we reached end of count! The datagram closing reason was " 
                  << lastReasonDataGram << " and the datagram closing action was " << lastActionDataGram << std::endl;
      }
    }
    m_digiEvent->Clear();
  }

  if (lastDatagramEventSiu1 != -1) {
    m_digiBranch->GetEntry(lastDatagramEventSiu1);
    if (m_digiEvent->getMetaEvent().datagram().crate() != enums::Lsf::Siu1) {
      std::cout << "   " << std::endl; 
      std::cout << "Anders! You fucked up! This should have been been from Siu1, but it's not! It is " << m_digiEvent->getMetaEvent().datagram().crate() << std::endl;
    } else {
      m_endCountDataGramSiu1 = 0;
      m_fullDataGramSiu1     = 0;
      int lastReasonDataGram = m_digiEvent->getMetaEvent().datagram().closeReason();
      int lastActionDataGram  = m_digiEvent->getMetaEvent().datagram().closeAction();
      if (lastReasonDataGram == enums::Lsf::Close::CountedOut) {
        m_endCountDataGramSiu1 = 1;
      }
      if (lastReasonDataGram == enums::Lsf::Close::Full) {
        m_fullDataGramSiu1 = 1;
      }
      if (m_fullDataGramSiu1==0 && m_endCountDataGramSiu1==0) {
  	std::cout << "   " << std::endl; 
        std::cout << "Warning! The last datagram for SIU1 was not closed because it was full or because we reached end of count! The datagram closing reason was " 
                  << lastReasonDataGram << " and the datagram closing action was " << lastActionDataGram << std::endl;
      }
    }
    m_digiEvent->Clear();
  }


  // Closing time:  
  if(m_mcFile) m_mcFile->Close();
  if(m_reconFile) m_reconFile->Close();
  if(m_digiFile) m_digiFile->Close();
}




void TestReport::analyzeReconTree()
{
  TkrRecon* tkrRecon = m_reconEvent->getTkrRecon();

  bool tkrReconSucceed = false;
  bool muonThroughCal = false;
 
  TVector3 pos, dir;

  Int_t nTk = 0;

  if(tkrRecon) {
    nTk = tkrRecon->getTrackCol()->GetLast()+1;
    m_nTkrTrack->Fill(tkrRecon->getTrackCol()->GetLast()+1);

    TObjArray* vertices = tkrRecon->getVertexCol();
    if(tkrRecon->getVertexCol()->GetLast() >= 0) {
      tkrReconSucceed = true;
      TkrVertex* tkrVertex = dynamic_cast<TkrVertex*>(vertices->At(0));
      if(tkrVertex) {
	pos = tkrVertex->getPosition();
	dir = tkrVertex->getDirection();
	m_reconDirXY->Fill(dir.X(), dir.Y());
	m_reconDirZ->Fill(dir.Z());
	m_reconPosXY->Fill(pos.X(), pos.Y());
	m_reconPosZ->Fill(pos.Z());
	m_reconEne->Fill(tkrVertex->getEnergy());
      }
    }
  }

  CalRecon* calRecon = m_reconEvent->getCalRecon();

  TVector3 calPos;

  if(calRecon) {

    TObjArray* calClusterCol = calRecon->getCalClusterCol();
    if(calClusterCol) {
      // currently there is just one cluster in CAL
      CalCluster* calCluster = dynamic_cast<CalCluster*>(calClusterCol->At(0));
      if(calCluster) {
	if(calCluster->getParams().getEnergy() > 50)  muonThroughCal = true;
	m_calSumEne->Fill(calCluster->getParams().getEnergy());
	calPos = calCluster->getParams().getCentroid();
      }
    }

    //condition variables to indicate whether a CAL layer had nonzero energy
    bool hasEne[g_nTower][g_nCalLayer];
    bzero(hasEne, g_nTower*g_nCalLayer*sizeof(bool));

    TObjArray* calXtalCol = calRecon->getCalXtalRecCol();
    if(calXtalCol) {
      for(int i = 0; i <= calXtalCol->GetLast(); ++i) {
	if(CalXtalRecData* p = 
	   dynamic_cast<CalXtalRecData*>(calXtalCol->At(i))) {
	  CalXtalId id = p->getPackedId();
	  int tower = id.getTower();
	  int layer = id.getLayer();
	  m_calEneLayer[tower][layer] += p->getEnergy();
	  hasEne[tower][layer] = 1;
	}
      }

      for(int i = 0; i != g_nTower; ++i) {
	for(int j = 0; j != g_nCalLayer; ++j) {
	  if(hasEne[i][j]) ++m_nCalEneLayer[i][j];
	}
      }
    }
  }

  if(tkrReconSucceed && muonThroughCal) {

    double x = dir.x()/dir.z()*(calPos.z()-pos.z()) + pos.x();
    double y = dir.y()/dir.z()*(calPos.z()-pos.z()) + pos.y();
    double distance = sqrt((x-calPos.x())*(x-calPos.x()) + (y-calPos.y())*(y-calPos.y()));

    m_alignCalTkr->Fill(distance);
  }

  // AcdRecon stuff
  AcdRecon* acdRecon = m_reconEvent->getAcdRecon();

  if ( acdRecon && nTk == 1) {

    // map of the acd hits
    std::map<int,const AcdHit*> acdHitMap;
    UInt_t nAcdHit = acdRecon->nAcdHit();
    for ( UInt_t iAcdHit(0); iAcdHit < nAcdHit; iAcdHit++ ) {
      const AcdHit* acdHit = acdRecon->getAcdHit(iAcdHit);
      acdHitMap[acdHit->getId().getId()] = acdHit;
    }
    
    UInt_t nAcdInter = acdRecon->nAcdIntersections();
    for ( UInt_t iAcdInter(0); iAcdInter < nAcdInter; iAcdInter++ ) {
      const AcdTkrIntersection* acdInter = acdRecon->getAcdTkrIntersection(iAcdInter);
      if ( acdInter->getTrackIndex() != 0 ) continue;
      //if ( acdInter->getArcLengthToIntersection() < 0 ) continue;
      UShort_t acdGemId = getGemId( acdInter->getTileId().getId() );
      if ( acdInter->tileHit() ) {
	m_AcdEfficMap->Fill( acdGemId );
	const AcdHit* acdHit = acdHitMap[ acdInter->getTileId().getId() ];	
	if ( acdHit != 0 ) {
	  Double_t angleToPlane = acdInter->getCosTheta();
	  Double_t mipsA = acdHit->getMips(AcdHit::A) * angleToPlane;
	  Double_t mipsB = acdHit->getMips(AcdHit::B) * angleToPlane;
	  m_AcdMipMapA->Fill(acdGemId,mipsA);
	  m_AcdMipMapB->Fill(acdGemId,mipsB);
	}
      } else {
	m_AcdInEfficMap->Fill( acdGemId );
	const TVector3& missPos = acdInter->getGlobalPosition();
	UInt_t face = acdInter->getTileId().getFace();
	switch ( face ) {
	case 0:
	  m_AcdMissMapTop->Fill( missPos.X(), missPos.Y() );
	  break;
	case 1:
	  m_AcdMissMapMinusX->Fill( missPos.Y(), missPos.Z() );
	  break;
	case 2: 
	  m_AcdMissMapMinusY->Fill( missPos.X(), missPos.Z() );
	  break;
	case 3:
	  m_AcdMissMapPlusX->Fill( missPos.Y(), missPos.Z() );
	  break;
	case 4: 
	  m_AcdMissMapPlusY->Fill( missPos.X(), missPos.Z() );
	  break;
	}
      }
    }
  }
}


void TestReport::analyzeDigiTree()
{
  unsigned int word = m_digiEvent->getL1T().getTriggerWord();
  unsigned bitMask = 0;
  int ibit = enums::number_of_trigger_bits;
  while(ibit--) { bitMask |= 1<<ibit; }
  int trigger = word & bitMask;
  m_trigger->Fill(trigger);
  for(int i = 0; i != enums::number_of_trigger_bits; ++i) {
    if( (trigger >> i) & 1) ++m_nEvtGltTrigger[i];
  }

  // LATTE run?
  m_isLATTE = 0;
  if (m_digiEvent->getEventSummaryData().oswLength() > 0) m_isLATTE = 1;

  if(m_digiEvent->getEventSummaryData().badEvent()) ++m_nBadEvts;

  // Error flags:
  if (m_digiEvent->getEventSummaryData().badEventSequence())  ++m_eventBadEventSequence;
  if (m_digiEvent->getEventSummaryData().badTkrRecon())       ++m_eventBadTkrRecon;
  if (m_digiEvent->getEventSummaryData().packetError())       ++m_eventPacketError;
  if (m_digiEvent->getEventSummaryData().temError())          ++m_eventTemError;
  if (m_digiEvent->getEventSummaryData().trgParityError())    ++m_eventTrgParityError;
  if (m_digiEvent->getEventSummaryData().badLdfStatus())      ++m_eventBadLdfStatus;
  if (m_digiEvent->getEventSummaryData().gtrcPhase())         ++m_eventGtrcPhase;
  if (m_digiEvent->getEventSummaryData().gtfePhase())         ++m_eventGtfePhase;
  if (m_digiEvent->getEventSummaryData().gtccFifo())          ++m_eventGtccFifo;
  if (m_digiEvent->getEventSummaryData().gtccHdrParity())     ++m_eventGtccHdrParity;
  if (m_digiEvent->getEventSummaryData().gtccWcParity())      ++m_eventGtccWcParity;
  if (m_digiEvent->getEventSummaryData().gtrcSummary())       ++m_eventGtrcSummary;
  if (m_digiEvent->getEventSummaryData().gtccDataParity())    ++m_eventGtccDataParity;
  if (m_digiEvent->getEventSummaryData().gtccTimeout())       ++m_eventGtccTimeout;
  if (m_digiEvent->getEventSummaryData().gcccError())         ++m_eventGcccError;
  if (m_digiEvent->getEventSummaryData().gtccError())         ++m_eventGtccError;
  if (m_digiEvent->getEventSummaryData().phaseError())        ++m_eventPhaseError;
  if (m_digiEvent->getEventSummaryData().timeoutError())      ++m_eventTimeoutError;




  int cond = m_digiEvent->getGem().getConditionSummary();
  m_condSummary->Fill(cond);
  for(int i = 0; i != enums::GEM_offset; ++i) {
    if( (cond >> i) & 1) {
      ++m_nEvtGemTrigger[i];
    }
  }

  // Periodic trigger?
  if ((m_digiEvent->getGem().getConditionSummary()) & 32) {
    m_eventIsPeriodic = 1;
  } else {
    m_eventIsPeriodic = 0;
    m_nEventNoPeriodic++;
  }

  // Readout modes:
  if (m_digiEvent->getEventSummaryData().readout4()==0 && m_digiEvent->getEventSummaryData().zeroSuppress()==1) {
    m_nbrEventsNormal++;
  }
  if (m_digiEvent->getEventSummaryData().readout4()==1 && m_digiEvent->getEventSummaryData().zeroSuppress()==1) {
    m_nbrEvents4Range++;
  }
  if (m_digiEvent->getEventSummaryData().readout4()==1 && m_digiEvent->getEventSummaryData().zeroSuppress()==0) {
    m_nbrEvents4RangeNonZS++;
  }


  // Conditions arrival time: Take out periodic triggers!
  if (!(m_digiEvent->getGem().getConditionSummary() & 32)) {
    if (m_digiEvent->getGem().getConditionSummary() & 2) {
      m_condArrivalTimeTKR->Fill(m_digiEvent->getGem().getCondArrTime().tkr());
    }
    if (m_digiEvent->getGem().getConditionSummary() & 1) {
      m_condArrivalTimeROI->Fill(m_digiEvent->getGem().getCondArrTime().roi());
    }
    if (m_digiEvent->getGem().getConditionSummary() & 4) {
      m_condArrivalTimeCalLo->Fill(m_digiEvent->getGem().getCondArrTime().calLE());
    }
    if (m_digiEvent->getGem().getConditionSummary() & 8) {
      m_condArrivalTimeCalHi->Fill(m_digiEvent->getGem().getCondArrTime().calHE());
    }
    if (m_digiEvent->getGem().getConditionSummary() & 16) {
      m_condArrivalTimeCNO->Fill(m_digiEvent->getGem().getCondArrTime().cno());
    }
  }

  // Triggers per tower:
  for (int i = 0; i != g_nTower; ++i) {
    m_tkrPerTowerArray[i]   = 0;
    m_calLoPerTowerArray[i] = 0;
    m_calHiPerTowerArray[i] = 0;
  }

  int gemTkrVector   = m_digiEvent->getGem().getTkrVector();
  int gemCalLoVector = m_digiEvent->getGem().getCalLeVector();
  int gemCalHiVector = m_digiEvent->getGem().getCalHeVector();

  for (int i = 0; i != g_nTower; ++i) {
    if ((gemTkrVector >> i)   & 1 ) ++m_tkrPerTowerArray[i];
    if ((gemCalLoVector >> i) & 1 ) ++m_calLoPerTowerArray[i];
    if ((gemCalHiVector >> i) & 1 ) ++m_calHiPerTowerArray[i];
  }
  for (int i = 0; i != g_nTower; ++i) {
    if (m_tkrPerTowerArray[i] > 0) {
      m_tkrPerTower->Fill(i);
    }
    if (m_calLoPerTowerArray[i] > 0) {
      m_calLoPerTower->Fill(i);
    }
    if (m_calHiPerTowerArray[i] > 0) {
      m_calHiPerTower->Fill(i);
    }
  }


  // GEM delta event time:
  // 1 count = 50 ns
  UInt_t deltaT = m_digiEvent->getGem().getDeltaEventTime();
  m_timeIntervalGem->Fill(0.00005*deltaT);

  if (deltaT < 1500) {
    m_timeIntervalGemZoom->Fill(deltaT);
  }

  // Delta window open time:
  UInt_t deltaWindowOpenTime = m_digiEvent->getGem().getDeltaWindowOpenTime();
  m_deltaWindowOpenTime->Fill(0.00005*deltaWindowOpenTime);

  if (deltaWindowOpenTime < 1500) {
    m_deltaWindowOpenTimeZoom->Fill(deltaWindowOpenTime);
  }

  // Ticks between 1-PPS:
  Int_t deltaTick = m_digiEvent->getMetaEvent().time().current().timeHack().ticks() - m_digiEvent->getMetaEvent().time().previous().timeHack().ticks();
  int RollOverInt = 33554432;  
  if (deltaTick < 0) {
    deltaTick = deltaTick + RollOverInt;
  }
  deltaTick       = deltaTick - 20000000;
  m_tick20MHzDeviation->Fill(deltaTick);
  if (deltaTick < 200 && deltaTick > -200) {
    m_tick20MHzDeviationZoom->Fill(deltaTick);
  }


  int tkrVector = m_digiEvent->getGem().getTkrVector();

  // 3 in a row TKR trigger
  bool tkrTrigger;
  if(cond & 2) {
    ++m_nTkrTrigger;
    tkrTrigger = true;

    for(int i = 0; i != g_nTower; ++i) {
      if( (tkrVector >> i) & 1 ) ++m_nTkrEvent[i];
    }
  }
  else {
    tkrTrigger = false;
  }

  int nTkrDigis = m_digiEvent->getTkrDigiCol()->GetLast()+1;

  int nHit[g_nTower], nPlane[g_nTower], nDigi[g_nTower];
  for(int i = 0; i != g_nTower; ++i) {
    nHit[i] = 0;
    nPlane[i] = 0;
    nDigi[i] = 0;
  }

  bool badStrip = false, moreStrip = false;
  bool satTot = false, zeroTot = false, badTot = false;

  for(int i = 0; i != nTkrDigis; ++i) {

    const TkrDigi* tkrDigi = m_digiEvent->getTkrDigi(i);
    assert(tkrDigi != 0); 

    int tower = tkrDigi->getTower().id();
    int biLayer = tkrDigi->getBilayer();
    GlastAxis::axis view = tkrDigi->getView();
    int iView = (view == GlastAxis::X) ? 0 : 1;
    int plane = Geo::instance()->getPlane(biLayer, iView);

    // Bay 10, layer split:
    if (tower==10 && plane==0) {
      if (tkrDigi->getLastController0Strip() != -1) {
        m_bay10Layer0SplitDefault = 1;
      }
    } 


    ++nDigi[tower];

    ++nPlane[tower];

    nHit[tower] += tkrDigi->getNumHits();

    m_nHitPlane[tower][plane] += tkrDigi->getNumHits();
    if(m_nHitPlane[tower][plane] > 0) ++m_nEvtPlane[tower][plane];

    int lowCount = 0, highCount = 0;
    int nStrips = tkrDigi->getNumHits();

    int lastController0Strip = tkrDigi->getLastController0Strip();

    for(int i = 0; i != nStrips; ++i) {
      int stripId = tkrDigi->getStrip(i);

      if(stripId < 0 || stripId >= g_nStrip) {
	badStrip = true;
      }
      else { 
	(stripId<=lastController0Strip) ? ++lowCount : ++highCount;
      }
    }

    if(lowCount > 0) ++m_nEvtHitPlane[tower][plane][0];
    if(highCount > 0) ++m_nEvtHitPlane[tower][plane][1];

    if(lowCount > 64 || highCount > 64) {
      moreStrip = true;
    }

    int tot0 = tkrDigi->getToT(0);
    int tot1 = tkrDigi->getToT(1);

    if((tot0>0  && lowCount==0) || (tot1>0  && highCount==0)) badTot  = true;
    if((tot0==0 && lowCount>0)  || (tot1==0 && highCount>0))  zeroTot = true;
    
    if (tot0<0 || tot0>g_overlapTot || tot1<0 || tot1>g_overlapTot) ++m_nEvtInvalidTot; 
    if (tot0>g_satTot && tot0!=g_overlapTot) ++m_nEvtInvalidTot; 
    if (tot1>g_satTot && tot1!=g_overlapTot) ++m_nEvtInvalidTot; 

    if(tot0==g_satTot || tot1==g_satTot) ++m_nEvtOverlapTriggerTot; 

    if(tot0 == 0){
      m_totZero2D->Fill(plane+0.25, tower, 1./m_nEvent);
      if(lowCount > 0) m_totNoise2D->Fill(plane+0.25, tower);
    }
    if(tot1 == 0){
      m_totZero2D->Fill(plane+0.75, tower, 1./m_nEvent);
      if(highCount > 0) m_totNoise2D->Fill(plane+0.75, tower);
    }

    if(tot0 == g_satTot){
      satTot = true;
      m_totSat2D->Fill(plane+0.25, tower);
    }
    if(tot1 == g_satTot){
      satTot = true;
      m_totSat2D->Fill(plane+0.75, tower);
    }

    if(tot0>0 && tot0<g_satTot) {
      m_tot[tower][plane][0]->Fill(tot0);
    }
    if(tot1>0 && tot1<g_satTot) {
      m_tot[tower][plane][1]->Fill(tot1);
    }

  }

  if(badStrip)  ++m_nEventBadStrip;
  if(moreStrip) ++m_nEventMoreStrip;
  if(badTot)    ++m_nEventBadTot;
  if(zeroTot)   ++m_nEventZeroTot;
  if(satTot)    ++m_nEventSatTot;

  int maxNDigi = 0;
  int minNDigi = 37;

  for(int i = 0; i != g_nTower; ++i) {
    m_nHit[i]->Fill(nHit[i]);
    m_nLayer[i]->Fill(nPlane[i]);
    if(nDigi[i] > maxNDigi) maxNDigi = nDigi[i];
    if( ((tkrVector >> i) & 1) && nDigi[i] < 6) ++m_nTkrBadEvent[i];
    if ( ((tkrVector >> i) & 1) && nDigi[i] < 6 && nDigi[i]<minNDigi) minNDigi = nDigi[i];
  }

  if(tkrTrigger) {
    // We really, really, really want the smallest number of digis and still a TKR trigger!
    //(maxNDigi >= 6) ? ++m_nEventDigi[6] : ++m_nEventDigi[maxNDigi];
    (minNDigi >= 6) ? ++m_nEventDigi[6] : ++m_nEventDigi[minNDigi];
  }

  // condition variables to indicate whether a particular layer has any hits
  int isHit[g_nTower][g_nCalLayer];
  bzero(isHit, g_nTower*g_nCalLayer*sizeof(int));

  //  int nCalDigis = m_digiEvent->getCalDigiCol()->GetLast()+1;
  const TClonesArray* calDigiCol = m_digiEvent->getCalDigiCol();
  for(int i = 0; i <= calDigiCol->GetLast(); ++i) {
    if(CalDigi* p = dynamic_cast<CalDigi*>(calDigiCol->At(i))) {
      int tower = p->getPackedId().getTower();
      int layer = p->getPackedId().getLayer();

      if (m_eventIsPeriodic == 0) {
        ++m_nCalHit[tower][layer];
        isHit[tower][layer] = 1;
      }
    }
  }

  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nCalLayer; ++j) {
      if(isHit[i][j]) ++m_nEvtCalHit[i][j];
    }
  }


  //
  // ACD digis:
  //
  int tmpAcdOddParityError    = 0;
  int tmpAcdHeaderParityError = 0;

  const TObjArray* acdDigiCol = m_digiEvent->getAcdDigiCol();
  assert(acdDigiCol != 0);

  int nAcdDigis = acdDigiCol->GetLast()+1;
  m_nAcdDigis->Fill(nAcdDigis);

  for(int iDigi = 0; iDigi != nAcdDigis; ++iDigi) {

    const AcdDigi* acdDigi = dynamic_cast<const AcdDigi*>(acdDigiCol->At(iDigi));
    assert(acdDigi != 0);

    int AcdGemID = getGemId ( acdDigi->getId().getId() );

    // add to hit map
    m_AcdHitMap->Fill(AcdGemID);

    // Get the hit map & veto flags
    Bool_t hitPmtA = acdDigi->getLowDiscrim(AcdDigi::A);
    Bool_t hitPmtB = acdDigi->getLowDiscrim(AcdDigi::B);
    Bool_t singlePmt = (hitPmtA && ! hitPmtB) || (!hitPmtA && hitPmtB);
    Bool_t vetoA = acdDigi->getVeto(AcdDigi::A);
    Bool_t vetoB = acdDigi->getVeto(AcdDigi::B);
    Bool_t singleVeto = (vetoA && !vetoB) || (!vetoA && vetoB);

    unsigned garcA(0); unsigned gafeA(0);
    unsigned garcB(0); unsigned gafeB(0);

    AcdId::convertToGarcGafe( acdDigi->getId().getId(), AcdDigi::A, garcA, gafeA );
    AcdId::convertToGarcGafe( acdDigi->getId().getId(), AcdDigi::B, garcB, gafeB );

    if ( hitPmtA && garcA < 12 ) m_AcdGarcGafeHitMap->Fill(gafeA,garcA);
    if ( hitPmtB && garcB < 12 ) m_AcdGarcGafeHitMap->Fill(gafeB,garcB);

    if ( vetoA && garcA < 12 ) m_AcdGarcGafeVetoMap->Fill(gafeA,garcA);
    if ( vetoB && garcB < 12 ) m_AcdGarcGafeVetoMap->Fill(gafeB,garcB);

    if ( singlePmt ) m_AcdTileIdOnePMT->Fill(AcdGemID);
    if ( singleVeto ) m_AcdTileIdOneVeto->Fill(AcdGemID);
    if ( vetoA || vetoB ) m_AcdVetoMap->Fill(AcdGemID);

    if (acdDigi->getRange(AcdDigi::A) == 0 ) m_AcdPhaMapA->Fill(AcdGemID, acdDigi->getPulseHeight(AcdDigi::A) );
    if (acdDigi->getRange(AcdDigi::B) == 0 ) m_AcdPhaMapB->Fill(AcdGemID, acdDigi->getPulseHeight(AcdDigi::B) );      
    
    if (acdDigi->getOddParityError(AcdDigi::A)) ++tmpAcdOddParityError;
    if (acdDigi->getOddParityError(AcdDigi::B)) ++tmpAcdOddParityError;

    if (acdDigi->getHeaderParityError(AcdDigi::A)) ++tmpAcdHeaderParityError;
    if (acdDigi->getHeaderParityError(AcdDigi::B)) ++tmpAcdHeaderParityError;
  }

  // ACD stuff in GEM
  const Gem& gem = m_digiEvent->getGem();
  const GemTileList& gemTL = gem.getTileList();
  for ( UInt_t iGemIdx(0); iGemIdx < 16; iGemIdx++ ) {
    // unfold the contribuions into a single vector
    if ( gemTL.getXzm() & ( 1 << iGemIdx ) )     m_AcdGemVetoMap->Fill(iGemIdx);
    if ( gemTL.getXzp() & ( 1 << iGemIdx ) )     m_AcdGemVetoMap->Fill(iGemIdx+16);
    if ( gemTL.getYzm() & ( 1 << iGemIdx ) )     m_AcdGemVetoMap->Fill(iGemIdx+32);
    if ( gemTL.getYzp() & ( 1 << iGemIdx ) )     m_AcdGemVetoMap->Fill(iGemIdx+48);
    if ( gemTL.getXy() & ( 1 << iGemIdx ) )      m_AcdGemVetoMap->Fill(iGemIdx+64);
    if ( gemTL.getXy() & ( 1 << (16+iGemIdx) ) ) m_AcdGemVetoMap->Fill(iGemIdx+80);
    if ( gemTL.getRbn() & ( 1 << iGemIdx ) )     m_AcdGemVetoMap->Fill(iGemIdx+96);
    if ( gemTL.getNa() & ( 1 << iGemIdx ) )      m_AcdGemVetoMap->Fill(iGemIdx+112);
  }  

  UShort_t roiVect = gem.getRoiVector();
  for ( UInt_t iRoiIdx(0); iRoiIdx < 16; iRoiIdx++ ) {
    if ( roiVect & ( 1 << iRoiIdx ) ) m_AcdGemRoiMap->Fill(iRoiIdx);
  } 

  UShort_t cnoVect = gem.getCnoVector();
  for ( UInt_t iCnoIdx(0); iCnoIdx < 12; iCnoIdx++ ) {
    if ( cnoVect & ( 1 << iCnoIdx ) ) m_AcdGemCnoMap->Fill(iCnoIdx);
  } 

  if (tmpAcdOddParityError)    ++m_nAcdOddParityError;
  if (tmpAcdHeaderParityError) ++m_nAcdHeaderParityError;

}

void TestReport::generateReport()
{
  writeHeader();

  (*m_report) << "@section purpose Purpose" << endl;
  (*m_report) << "This report is used in offline data analyses to identify apparent problems in cosmic ray muon and VDG data. <b>Warning! Results from other tests(such as charge injection) should be interpreted with care.</b>" << endl;

  (*m_report) << "@section version Software Version" << endl;
  (*m_report) << "@li EngineeringModel: @b " << m_emVersion << endl;
  (*m_report) << "@li TestReport: @b " << m_version << endl;
  /*
  (*m_report) << "@section calibversion Serial no. of calibration constants (-9999 means no constants were used.)" << endl;
  (*m_report) << "@li TKR: @b " << m_tkrCalibSerNo << endl;
  (*m_report) << "@li CAL: @b " << m_calCalibSerNo << endl;
  */
  (*m_report) << "@section summary Summary" << endl;
  if(m_nEvent == 0) {
    (*m_report) << "<b> There are no events in this run! </b>" << endl;
    return;
  } 


  if (m_digiFile) {
    (*m_report) << "In the digi file @em " << m_digiFile->GetName() << endl;
  }
  if (m_digiFile && m_reconFile) {
    (*m_report) << " and " << endl;
  }
  if (m_reconFile) {
    (*m_report) << "In the recon file @em " << m_reconFile->GetName() << endl;
  }


  if (m_isLATTE == 1) {
    (*m_report) << "@li There are @b " << m_nEvent << " triggers. This run was taken with LATTE so there will be " << m_nEvent+2 << " events recorded in the eLog database since LATTE adds two additional events in the process which are not triggered events." << endl;
  } else {
    (*m_report) << "@li There are @b " << m_nEvent << " triggers." << endl;
  }
  (*m_report) << "   " << endl;


  if (m_digiFile) {
    (*m_report) << "@li Time of the first trigger: <b>" << asctime((struct tm*) (gmtime((time_t*) (&m_startTime)))) << " (GMT) </b>";
    (*m_report) << "@li Time of the last trigger: <b>" << asctime((struct tm*) (gmtime((time_t*) (&m_endTime)))) << " (GMT) </b>";
    (*m_report) << "@li Duration: <b>" << m_endTime - m_startTime << " seconds" << "</b>" << endl;


    m_report->setf(ios::fixed);
    (*m_report) << "@li Trigger rate: <b>" <<std::setprecision(1) <<  double(m_nEvent)/ double (m_endTime - m_startTime) << " Hz" << "</b>" << endl;
    (*m_report) << "@li Livetime corrected trigger rate: <b>" << (double(m_nEvent) / double (m_endTime - m_startTime)) / m_liveTime << " Hz" << "</b>" << endl;
    (*m_report) << "   " << endl;

    if (m_counterGroundID == 0) {
      (*m_report) << "@li The Ground ID is <b>" << m_firstGroundID << "</b>" << endl;
    } else {
      (*m_report) << "@li Warning! The ground ID changed @b " << m_counterGroundID << " times during the run! The first event had ground ID @b " << m_firstGroundID << " while the last event had ground ID @b " << m_lastGroundID << ". See the log file for more details. " << endl;
    }
    (*m_report) << "   " << endl;


    if (m_bay10Layer0SplitDefault == 1) {
      (*m_report) << "   " << endl;
      (*m_report) << "@li Warning: We are reading out hits from the left in TKR Bay 10, layer 0! Is this intentional?" << endl;
      (*m_report) << "   " << endl;
    }

    // Needs Spectrum Astro FSW!:
    (*m_report) << "@li Livetime: <b> " << (m_liveTime * 100.0) << "% </b>" << endl;  

    if (m_deltaSequenceNbrEvents == 0) {
      (*m_report) << "@li Expected livetime from event readout categories: <b> " <<  (1.0 - ((m_nbrEventsNormal*529.0 + m_nbrEvents4Range*1318.0 + m_nbrEvents4RangeNonZS*12500.0)/ double (m_elapsedTime))) * 100.0 << "%</b>. This estimate is only valid if the filter is not running!" << std::endl;
    }

    if ((m_nbrEventsNormal+m_nbrEvents4Range+m_nbrEvents4RangeNonZS) != m_nEvent) {
      std::cout << "Problem!!!!! " << (m_nbrEventsNormal+m_nbrEvents4Range+m_nbrEvents4RangeNonZS) << "   " << m_nEvent << "   " << m_nbrEventsNormal << "   " << m_nbrEvents4Range << "   " 
                << m_nbrEvents4RangeNonZS << std::endl; 
    }

    (*m_report) << "   " << endl;
    (*m_report) << "@li There were @b " << m_nbrPrescaled << " prescaled events (<b>" << double (m_nbrPrescaled)/ double ((m_endTime - m_startTime)) <<" Hz</b>), @b " << m_nbrDeadZone  << " dead zone events (<b>" << double (m_nbrDeadZone)/ double ((m_endTime - m_startTime)) <<" Hz</b>) and @b " << m_nbrDiscarded << " discarded events (<b>" << double (m_nbrDiscarded)/ double ((m_endTime - m_startTime))  <<" Hz</b>)." << endl;


    if (m_deltaSequenceNbrEvents != 0) {
      (*m_report) << "@li The number of events in the digi file does not agree with the extended GEM sequence counter! The difference is @b " << m_deltaSequenceNbrEvents << " events. Is the onboard filter running?" << endl;
    } else {
      (*m_report) << "@li The number of events in the digi file agrees with the extended GEM sequence counter!" << endl;
    }

    (*m_report) << "   " << endl;
    (*m_report) << "@li There were @b " << m_nbrMissingTimeTone << " events with a missing Time tone, @b " << m_nbrFlywheeling << " flywheeling events, @b " << m_nbrIncomplete << " events with an incomplete time tone, @b " << m_nbrMissingGps << " events with a missing GPS lock, @b " << m_nbrMissingCpuPps << " events with a missing 1-PPS signal at CPU level and @b " << m_nbrMissingLatPps << " events with a missing 1-PPS signal at LAT level." << endl; 

    if (m_extendedCountersFlag != 0) {
      (*m_report) << "@li Problem! At least one of the extended counters decreased from one event to the next one  @b " << m_extendedCountersFlag << " times! Check the log file for more details." << endl;
    } 

    if (m_backwardsTimeTone != 0) {
      (*m_report) << "@li Problem! Some of the time tones seem to go backwards from one event to the next one. It happened @b " << m_backwardsTimeTone << " times! Check the log file for more details." << endl;
    }   

    if (m_identicalTimeTones != 0) {
      (*m_report) << "@li Problem! Current and previous time tones are identical in some events. It happened @b " << m_identicalTimeTones << " times! Check the log file for more details." << endl;
    }


    (*m_report) << "   " << endl;

 
    if (m_nbrDataGramsEpu0 > 0) {
      (*m_report) << "@li There were @b " << m_nbrDataGramsEpu0 << " datagrams from EPU0 in this run with in average <b> " << ((float) m_nbrEventsDataGramsEpu0 / (float) m_nbrDataGramsEpu0) << " </b> events per datagram." << endl;
    }
    if (m_nbrDataGramsEpu1 > 0) {
      (*m_report) << "@li There were @b " << m_nbrDataGramsEpu1 << " datagrams from EPU1 in this run with in average <b> " << ((float) m_nbrEventsDataGramsEpu1 / (float) m_nbrDataGramsEpu1) << " </b> events per datagram." << endl;
    }
    if (m_nbrDataGramsEpu2 > 0) {
      (*m_report) << "@li There were @b " << m_nbrDataGramsEpu2 << " datagrams from EPU2 in this run with in average <b> " << ((float) m_nbrEventsDataGramsEpu2 / (float) m_nbrDataGramsEpu2) << " </b> events per datagram." << endl;
    }
    if (m_nbrDataGramsSiu0 > 0) {
      (*m_report) << "@li There were @b " << (m_counterCyclesSiu0+1) << " cycles with a maximum of @b " << m_nbrDataGramsSiu0 << " datagrams per cycle from SIU0 in this run with in average <b> " << ((float) m_nbrEventsDataGramsSiu0 / ((float) m_nbrDataGramsSiu0 * (float) (m_counterCyclesSiu0+1.0))) << " </b> events per datagram." << endl;
    }
    if (m_nbrDataGramsSiu1 > 0) {
      (*m_report) << "@li There were @b " << (m_counterCyclesSiu1+1) << " cycles with a maximum of @b " << m_nbrDataGramsSiu1 << " datagrams per cycle from SIU1 in this run with in average <b> " << ((float) m_nbrEventsDataGramsSiu1 / ((float) m_nbrDataGramsSiu1 * (float) (m_counterCyclesSiu1+1.0))) << " </b> events per datagram." << endl;
    }



    // EPU0:
    if (m_nbrEventsDataGramsEpu0 > 0) {
      if (m_counterDataDiagramsEpu0 != 0) {
         (*m_report) << "@li Problem! We dropped  @b " << m_counterDataDiagramsEpu0 << " datagram(s) from EPU0 in this run!" << endl;
      }

      if (m_beginRunDataGramEpu0 != 1) {
        (*m_report) << "@li Problem! The first datagram in EPU0 was not the first datagram after the start of the run!" << endl;
      }
      if (m_firstDataGramEpu0 != 0) {
        (*m_report) << "@li Problem! The first datagram in EPU0 did not have sequence number 0! It was @b " << m_firstDataGramEpu0 << "." << endl;
      }
      if (m_endRunDataGramEpu0 == 0 && m_fullDataGramEpu0==1) {
        (*m_report) << "@li Problem! The last datagram from EPU0 was not closed because of end of run, but because it was full! Are we missing events?" << endl;
      }
      if (m_endRunDataGramEpu0 ==0 && m_fullDataGramEpu0==0) {
        (*m_report) << "@li Problem! The last datagram from EPU0 was not closed neither because of end of run neither because it was full. See logfile for more details!" << endl;
      }
    }

    // EPU1:
    if (m_nbrEventsDataGramsEpu1 > 0) {
      if (m_counterDataDiagramsEpu1 != 0) {
        (*m_report) << "@li Problem! We dropped  @b " << m_counterDataDiagramsEpu1 << " datagram(s) from EPU1 in this run!" << endl;
      }

      if (m_beginRunDataGramEpu1 != 1) {
        (*m_report) << "@li Problem! The first datagram in EPU1 was not the first datagram after the start of the run!" << endl;
      }
      if (m_firstDataGramEpu1 != 0) {
        (*m_report) << "@li Problem! The first datagram in EPU1 did not have sequence number 0! It was @b " << m_firstDataGramEpu1 << "." << endl;
      }
      if (m_endRunDataGramEpu1== 0 && m_fullDataGramEpu1==1) {
        (*m_report) << "@li Problem! The last datagram from EPU1 was not closed because of end of run, but because it was full! Are we missing events?" << endl;
      }
      if (m_endRunDataGramEpu1==0 && m_fullDataGramEpu1==0) {
        (*m_report) << "@li Problem! The last datagram from EPU1 was not closed neither because of end of run neither because it was full. See logfile for more details!" << endl;
      }
    }

    // EPU2:
    if (m_nbrEventsDataGramsEpu2 > 0) {
      if (m_counterDataDiagramsEpu2 != 0) {
        (*m_report) << "@li Problem! We dropped  @b " << m_counterDataDiagramsEpu2 << " datagram(s) from EPU2 in this run!" << endl;
      }

      if (m_beginRunDataGramEpu2 != 1) {
        (*m_report) << "@li Problem! The first datagram in EPU2 was not the first datagram after the start of the run!" << endl;
      }
      if (m_firstDataGramEpu2 != 0) {
        (*m_report) << "@li Problem! The first datagram in EPU2 did not have sequence number 0! It was @b " << m_firstDataGramEpu2 << "." << endl;
      }
      if (m_endRunDataGramEpu2== 0 && m_fullDataGramEpu2==1) {
        (*m_report) << "@li Problem! The last datagram from EPU2 was not closed because of end of run, but because it was full! Are we missing events?" << endl;
      }
      if (m_endRunDataGramEpu2==0 && m_fullDataGramEpu2==0) {
        (*m_report) << "@li Problem! The last datagram from EPU2 was not closed neither because of end of run neither because it was full. See logfile for more details!" << endl;
      }
    }

    // SIU0
    if (m_nbrEventsDataGramsSiu0 > 0) {
      if (m_counterDataDiagramsSiu0 != 0) {
        (*m_report) << "@li Problem! We dropped  @b " << m_counterDataDiagramsSiu0 << " datagram(s) from SIU0 in this run!" << endl;
      }

      if (m_beginRunDataGramSiu0 != 1) {
        (*m_report) << "@li Problem! The first datagram in SIU0 was not the first datagram after the start of the run!" << endl;
      }
      if (m_firstDataGramSiu0 != 0) {
        (*m_report) << "@li Problem! The first datagram in SIU0 did not have sequence number 0! It was @b " << m_firstDataGramSiu0 << "." << endl;
      }
      if (m_endCountDataGramSiu0==0 && m_fullDataGramSiu0==1) {
        (*m_report) << "@li Problem! The last datagram from SIU0 was not closed because of end of count, but because it was full! Are we missing events?" << endl;
      }
      if (m_endCountDataGramSiu0==0 && m_fullDataGramSiu0==0) {
        (*m_report) << "@li Problem! The last datagram from SIU0 was not closed neither because of end of count nor because it was full. See logfile for more details!" << endl;
      }
    }

    // SIU1
    if (m_nbrEventsDataGramsSiu1 > 0) {
      if (m_counterDataDiagramsSiu1 != 0) {
        (*m_report) << "@li Problem! We dropped  @b " << m_counterDataDiagramsSiu1 << " datagram(s) from SIU1 in this run!" << endl;
      }

      if (m_beginRunDataGramSiu1 != 1) {
        (*m_report) << "@li Problem! The first datagram in EPU1 was not the first datagram after the start of the run!" << endl;
      }
      if (m_firstDataGramSiu1 != 0) {
        (*m_report) << "@li Problem! The first datagram in SIU1 did not have sequence number 0! It was @b " << m_firstDataGramSiu1 << "." << endl;
      }
      if (m_endCountDataGramSiu1==0 && m_fullDataGramSiu1==1) {
        (*m_report) << "@li Problem! The last datagram from SIU1 was not closed because of end of count, but because it was full! Are we missing events?" << endl;
      }
      if (m_endCountDataGramSiu1==0 && m_fullDataGramSiu1==0) {
        (*m_report) << "@li Problem! The last datagram from SIU1 was not closed neither because of end of count nor because it was full. See logfile for more details!" << endl;
      }
    }


    // EPU0 datagram gaps:
    if (m_nbrEventsDataGramsEpu0 > 0) {
      if (m_datagramGapsEPU0 != 0) {
        (*m_report) << "   " << endl;
        (*m_report) << "@li Problem! There were @b " << m_datagramGapsEPU0 << " datagram sequence number gaps from EPU0 in this run! " ;
        if (m_counterDataDiagramsEpu0 == 0) {
          (*m_report) << "Since no datagrams were actually dropped this could be the 4.2s CCSDS time shift!" << endl;
        }
      }  
    }
    // EPU1 datagram gaps:
    if (m_nbrEventsDataGramsEpu1 > 0) {
      if (m_datagramGapsEPU1 != 0) {
        (*m_report) << "   " << endl;
        (*m_report) << "@li Problem! There were @b " << m_datagramGapsEPU1 << " datagram sequence number gaps from EPU1 in this run! ";
        if (m_counterDataDiagramsEpu1 == 0) {
          (*m_report) << "Since no datagrams were actually dropped this could be the 4.2s CCSDS time shift!" << endl;
        }
      }  
    }
    // EPU2 datagram gaps:
    if (m_nbrEventsDataGramsEpu2 > 0) {
      if (m_datagramGapsEPU2 != 0) {
        (*m_report) << "   " << endl;
        (*m_report) << "@li Problem! There were @b " << m_datagramGapsEPU2 << " datagram sequence number gaps from EPU2 in this run! " ;
        if (m_counterDataDiagramsEpu2 == 0) {
          (*m_report) << "Since no datagrams were actually dropped this could be the 4.2s CCSDS time shift!" << endl;
        }
      }  
    }
    // SIU0 datagram gaps:
    if (m_nbrEventsDataGramsSiu0 > 0) {
      if (m_datagramGapsSIU0 != 0) {
        (*m_report) << "   " << endl;
        (*m_report) << "@li Problem! There were @b " << m_datagramGapsSIU0 << " datagram sequence number gaps from SIU0 in this run! " ;
        if (m_counterDataDiagramsSiu0 == 0) {
          (*m_report) << "Since no datagrams were actually dropped this could be the 4.2s CCSDS time shift!" << endl;
        }
      }  
    }
    // SIU1 datagram gaps:
    if (m_nbrEventsDataGramsSiu1 > 0) {
      if (m_datagramGapsSIU1 != 0) {
        (*m_report) << "   " << endl;
        (*m_report) << "@li Problem! There were @b " << m_datagramGapsSIU1 << " datagram sequence number gaps from SIU1 in this run! " ;
        if (m_counterDataDiagramsSiu1 == 0) {
          (*m_report) << "Since no datagrams were actually dropped this could be the 4.2s CCSDS time shift!" << endl;
        }
      }  
    }


    (*m_report) << "   " << endl;

    (*m_report) << "@li There are @b " << m_nBadEvts              << " bad events as defined by Offline (catch all flag)." << endl;

    (*m_report) << "@li There are @b " << m_eventTrgParityError   << " events with Trigger Parity errors. " << endl;
    (*m_report) << "@li There are @b " << m_eventPacketError      << " events with Packet errors. " << endl;

    (*m_report) << "@li There are @b " << m_eventPhaseError       << " events with Phasing errors." << endl;
    (*m_report) << "@li There are @b " << m_eventTimeoutError     << " events with Timeout errors." << endl;

    (*m_report) << "@li There are @b " << m_nAcdOddParityError    << " events with ACD Odd Parity errors. " << endl;
    (*m_report) << "@li There are @b " << m_nAcdHeaderParityError << " events with ACD 'Header Parity errors' (there should _never_ be any)." << endl;

    (*m_report) << "@li There are @b " << m_eventBadLdfStatus     << " events with a bad LDF status flag. " << endl;

    (*m_report) << "@li There are @b " << m_eventBadEventSequence << " events with event sequence issues (not increasing monotonically)." << endl;

    (*m_report) << "@li There are @b " << m_eventTemError         << " events with TEM errors (includes TKR FIFO full errors)." << endl;

    (*m_report) << "@li There are @b " << m_eventGtccError        << " events with GTCC errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGtccFifo         << " events with GTCC FIFO errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGtccHdrParity    << " events with GTCC Header Parity errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGtccWcParity     << " events with GTCC Word Count Parity errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGtccDataParity   << " events with GTCC Data Parity errors." << endl; 
    (*m_report) << "@li There are @b " << m_eventGtccTimeout      << " events with GTCC Timeout errors." << endl;

    (*m_report) << "@li There are @b " << m_eventGtrcSummary      << " events with GTRC Summary errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGtrcPhase        << " events with GTRC Phase errors." << endl;

    (*m_report) << "@li There are @b " << m_eventGtfePhase        << " events with GTFE Phase errors." << endl;
    (*m_report) << "@li There are @b " << m_eventGcccError        << " events with GCCC errors." << endl;
    
    (*m_report) << "@li There are @b " << m_eventBadTkrRecon      << " events passing the Offline Bad TKR Recon criteria." << endl;
  }

  if(m_reconFile) {
    (*m_report) << "<p>The Recon file is: @em " << m_reconFile->GetName() << "</p>" << endl;
    generateReconReport();
  }

  if(m_digiFile) generateDigiReport();
 
  writeTail(); 
}

void TestReport::generateDigiReport()
{
  (*m_report) << "@section trigger Trigger" << endl;

  // print condition summary plots
  string file = m_prefix;
  file += "_condSummary";
  PlotAttribute att(file.c_str(), "GEM condition summary word. The word is deduced by combining bit patterns from the table shown below. For example, an event with both the TKR trigger bit and the CAL low trigger bit set in GEM has the condition summary word of @latex $2^{2} + 2^{1} = 6$ @endlatex @html 2<sup>2</sup> + 2<sup>1</sup> = 6 @endhtml", "condSummary", 1);
  att.m_statMode = 11;
  producePlot(m_condSummary, att);
  insertPlot(att);
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  printCondSummaryTable();

  // print trigger plots
  file = m_prefix;
  file += "_trigger";
  att.set(file.c_str(), "Trigger word calculated by triggerAlg. The word is deduced by combining bit patterns from the table shown below. For example, an event with both the TKR trigger bit and the CAL Low trigger bit set has the GLT word of @latex $2^{1} + 2^{2} = 6$ @endlatex @html 2<sup>1</sup> + 2<sup>2</sup> = 6 @endhtml.", "trigger", 1);
  att.m_statMode = 11;
  producePlot(m_trigger, att);
  insertPlot(att);
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  printGltTriggerTable();

  // Trigger rates:
  produceTriggerRatePlot();


  // Trigger per tower:
  produceTriggerPerTowerPlot();

  // Acd Trigger plots
  produceAcdTriggerPlots();

  // Arrival times:
  produceCondArrivalTimesPlots();

  // GEM discarded events:
  (*m_report) << "@section gemDiscarded GEM Discarded and DeadZone Events" << endl;
  produceGemDiscardedPlot();

  (*m_report) << "@section timeInfo Time Info" << endl;

  if (m_isLATTE == 1) {
    produceTimeIntervalPlotSBC();
  }
  produceTimeIntervalPlotGEM();


  // FSW stuff:
  (*m_report) << "@section epu Crate Number" << endl;
  produceEpuPlot();



  (*m_report) << "@section tkrDigi TKR Digitization" << endl;

  *(m_report) << "@subsection nDigi nDigi" << endl;
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  printNDigiTable();
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  printNDigiTowerTable();
  *(m_report) << "@latexonly \\pagebreak @endlatexonly" << endl;
  *(m_report) << "@latexonly \\pagebreak @endlatexonly" << endl;

  (*m_report) << "@subsection strip_hit Strip Hit Info" << endl;
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  *(m_report) << "@li There are @b " << ToString(m_nEventBadStrip) << " events with strip ID either outside the physical range from 0 to 1535 or outside the range read by the GTRCs." << endl;

  *(m_report) << "@li There are @b " << ToString(m_nEventMoreStrip) << " events with more than 64 strips per GTRC. ";

  *(m_report) << endl << endl;
  
  produceNhits2DPlot();

  //  produceDigiPlot();

  // produce plots for nhits distribution for 16 towers
  file = m_prefix;
  file += "_hitsPerTower_1";
  att.set(file.c_str(), "Number of strip hits in a tower(for tower 0 to tower 7).", "hitsPerTower_1", 1, 0, 18, 18, 909, 615);
  producePlot((TObject**) m_nHit, att, 4, 2);
  insertPlot(att);

  file = m_prefix;
  file += "_hitsPerTower_2";
  att.set(file.c_str(), "Number of strip hits in a tower(for tower 8 to tower 15.", "hitsPerTower_2", 1, 0, 18, 18, 909, 615);
  producePlot((TObject**) m_nHit+8, att, 4, 2);
  insertPlot(att);

  produceNHitPlane2DPlot();

  // produce plots for nLayer distribution

  file = m_prefix;
  file += "_layersPerTower_1";
  att.set(file.c_str(), "Number of planes that are hit in a tower(for tower 0 to tower 7).", "layersPerTower_1", 1, 0, 18, 18, 909, 615, 111);
  producePlot((TObject**) m_nLayer, att, 4, 2);
  insertPlot(att);

  file = m_prefix;
  file += "_layersPerTower_2";
  att.set(file.c_str(), "Number of planes that are hit in a tower(for tower 8 to tower 15).", "layersPerTower_2", 1, 0, 18, 18, 909, 615, 111);
  producePlot((TObject**) m_nLayer+8, att, 4, 2);
  insertPlot(att);

  *(m_report) << "@latexonly \\pagebreak @endlatexonly" << endl;
  *(m_report) << "@subsection totInfo Time Over Threshold (TOT) information" << endl;
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  *(m_report) << "@li Fraction of events with saturated TOT value of " << g_satTot << " counts(1 count = 200ns): @b " << ToString(m_nEventSatTot/double(m_nEvent)) << endl;
  *(m_report) << "@li Fraction of events with overlapped triggers i.e. TOT value of " << g_overlapTot << " counts(1 count = 200ns): @b " << ToString(m_nEvtOverlapTriggerTot/double(m_nEvent)) << endl;
  *(m_report) << "@li Fraction of events with invalid TOT (outside the range [0, " << g_satTot << "] and != g_overlapTot): @b " << ToString(m_nEvtInvalidTot/double(m_nEvent)) << endl;
  *(m_report) << "@li Fraction of events with TOT==0 and nStrip!=0 for each Si plane: @b " << ToString(m_nEventZeroTot/double(m_nEvent)) << endl;
  *(m_report) << "@li Fraction of events with TOT!=0 and nStrip==0 for each Si plane: @b " << ToString(m_nEventBadTot/double(m_nEvent)) << endl;

  // TOT plots
  produceTotNoise2DPlot();

  produceZeroSatTot2DPlot();

  produceAveTot2DPlot();
  //  produceAveTotPlots();

  (*m_report) << "@section calDigi CAL Digitization" << endl;

  produceCalNhits2DPlot();

  // ACD digis:
  (*m_report) << "@section acdDigi ACD Digitization" << endl;

  produceAcdDigiPlots();

}

void TestReport::generateReconReport()
{
  (*m_report) << "@section tkrRecon TKR Reconstruction" << endl;

  // print reconstructed track plots
  string file(m_prefix);
  file += "_nTkrTrack";
  PlotAttribute att(file.c_str(), "Number of reconstructed tracks for all trigger types. This data is obtained from track collections stored in the TkrRecon class.", "nTkrTrack", 1);
  producePlot(m_nTkrTrack, att);
  insertPlot(att);

  printNTrackTable();

  produceReconDirPlots();

  produceReconPosPlots();

  //  produceReconEnePlots();

  (*m_report) << "@section calRecon CAL Reconstruction" << endl;

  produceCalEneSum2DPlot();

  produceCalEneLayer2DPlot();

  (*m_report) << "@section align Alignment between TKR and CAL Reconstruction" << endl;

  produceAlignCalTkrPlot();

  (*m_report) << "@section acdTkrRecon Track extrapolation to ACD" << endl;

  produceAcdTkrReconPlots();

}

void TestReport::writeHeader()
{
  *(m_report) << "/** @mainpage SVAC Report" << endl
	      << "@htmlonly" << endl
	      << "<center>" << endl
	      << "<a href=\"../latex/refman.ps\"> PS file </a> &nbsp" << endl
	      << "<a href=\"../latex/refman.pdf\"> PDF file </a>" << endl 
	      << "</center>" << endl
	      << "@endhtmlonly" << endl
	      << "@author{automatically generated}" << endl << endl;
}

void TestReport::writeTail()
{
  *(m_report) << "*/" << endl;
}


void TestReport::producePlot(TObject* h, const PlotAttribute& att)
{
  TCanvas c1("c1", "produce eps file", att.m_xPixel, att.m_yPixel);
  gPad->SetBorderMode(0);
  gPad->SetFillColor(10);

  //set to default values
  gStyle->SetTitleH();
  gStyle->SetTitleW();
  gStyle->SetTitleFontSize();
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.16);
  gStyle->SetStatX(1.0);
  gStyle->SetStatY(1.0);
  gStyle->SetPalette();

  if(att.m_yLog) gPad->SetLogy();
  if(att.m_zLog) {
    cout << "set logz" << endl;
    gPad->SetLogz();
  }

  Double_t minY(0.);
  Double_t maxY(0.);

  if(TGraph* gr = dynamic_cast<TGraph*>(h)) {
    gr->Draw("A*");
  }
  else if(TH2F* h2 = dynamic_cast<TH2F*>(h)) {
    if(att.m_2dPlotType == PlotAttribute::LEGO) {
      h2->Draw("LEGO");
    }
    else {
      gStyle->SetPalette(1);
      //      gStyle->SetPalette(att.m_nColor, (int*) att.m_colors);
      gPad->SetRightMargin(0.15);
      // 
      if (att.m_statBox) {
        gStyle->SetStatW(0.15);
        gStyle->SetStatH(0.12);
        gStyle->SetStatX(0.84);
        gStyle->SetStatY(0.99);
      }
      gStyle->SetOptStat(att.m_statMode);
      //
      h2->Draw("COLZ");
      minY = h2->GetYaxis()->GetXmin();
      maxY = h2->GetYaxis()->GetXmax();
    }
  }
  else{
    TH1* h1 = dynamic_cast<TH1*>(h);
    if ( h1 != 0 ) {
      minY = h1->GetMinimum();
      maxY = h1->GetMaximum();
    }
    gStyle->SetOptStat(att.m_statMode);
    h->Draw();
  }

  // For some of TKR plots, draw additional lines and texts to distinguish
  // among no converter, thin converter and thick converter.

  // For unknown reasons, the following variables must be outside the if
  // scope in order for lines and texts to be drawn on the canvas  
  TLine l1(3.5, 15.5, 3.5, 16);
  TLatex tex1(-0.5, 15.6, "no W");
  TLine l2(11.5, 15.5, 11.5, 16);
  TLatex tex2(5, 15.6, "thick W");
  TLatex tex3(20, 15.6, "thin W");

  if(h == m_nHit2D || h == m_nZeroHit2D) {
    l1.Draw();
    tex1.Draw();
    l2.Draw();
    tex2.Draw();
    tex3.Draw();
  }


  // For all of the ACD Gem ID plots, draw additional lines and texts to distinguish
  // among top & sides

  // Because of the memory management of TLine reasons, the following variables must be outside the if
  // scope in order for lines and texts to be drawn on the canvas  
    
  TLine lAcd1(15.5,minY,15.5,maxY);
  TLine lAcd2(31.5,minY,31.5,maxY);
  TLine lAcd3(47.5,minY,47.5,maxY);
  TLine lAcd4(63.5,minY,63.5,maxY);
  TLine lAcd5(88.5,minY,88.5,maxY);
  TLine lAcd6(95.5,minY,95.5,maxY);
  TLine lAcd7(103.5,minY,103.5,maxY);

  TLatex texAcd1(5, maxY, "-Y");
  TLatex texAcd2(21, maxY, "+Y");
  TLatex texAcd3(37, maxY, "-X");
  TLatex texAcd4(53, maxY, "+X");
  TLatex texAcd5(69, maxY, "TOP");
  TLatex texAcd6(99, maxY, "Rb");  

  TList naList;
  TMarker marker0(1.,0.,3); naList.Add(&marker0);
  TMarker marker1(1.,6.,3); naList.Add(&marker1);  
  TMarker marker2(2.,8.,3); naList.Add(&marker2);
  TMarker marker3(3.,0.,3); naList.Add(&marker3);  
  TMarker marker4(3.,6.,3); naList.Add(&marker4);
  TMarker marker5(13.,3.,3); naList.Add(&marker5);  
  TMarker marker6(13.,4.,3); naList.Add(&marker6);
  TMarker marker7(13.,9.,3); naList.Add(&marker7);  
  TMarker marker8(13.,10.,3); naList.Add(&marker8);
  TMarker marker9(14.,1.,3); naList.Add(&marker9);

  TMarker marker10(14.,7.,3); naList.Add(&marker10);
  TMarker marker11(15.,9.,3); naList.Add(&marker11);  
  TMarker marker12(16.,1.,3); naList.Add(&marker12);
  TMarker marker13(16.,2.,3); naList.Add(&marker13);  
  TMarker marker14(16.,3.,3); naList.Add(&marker14);
  TMarker marker15(16.,4.,3); naList.Add(&marker15);  
  TMarker marker16(16.,5.,3); naList.Add(&marker16);
  TMarker marker17(16.,7.,3); naList.Add(&marker17);  
  TMarker marker18(16.,8.,3); naList.Add(&marker18);
  TMarker marker19(16.,9.,3); naList.Add(&marker19);
  
  TMarker marker20(16.,10.,3); naList.Add(&marker20);
  TMarker marker21(16.,11.,3); naList.Add(&marker21);

  if(h == m_AcdGemVetoMap ||
     h == m_AcdTileIdOnePMT ||
     h == m_AcdTileIdOneVeto ||
     h == m_AcdHitMap ||
     h == m_AcdVetoMap ||
     h == m_AcdPhaMapA ||
     h == m_AcdPhaMapB ||
     h == m_AcdInEfficMap ||
     h == m_AcdMipMapA || 
     h == m_AcdMipMapB ) {
    lAcd1.Draw(); lAcd2.Draw(); lAcd3.Draw(); lAcd4.Draw(); lAcd5.Draw(); lAcd6.Draw(); lAcd7.Draw(); 
    texAcd1.Draw(); texAcd2.Draw(); texAcd3.Draw(); texAcd4.Draw(); texAcd5.Draw(); texAcd6.Draw();
  }

  if ( h == m_AcdGarcGafeHitMap ||
       h == m_AcdGarcGafeVetoMap ) {
    naList.Draw();
  }

  // FIXME -- should draw the tile edges on the miss maps


  string epsFile(m_dir);
  epsFile += "/";
  epsFile += att.m_file;
  epsFile += ".eps";
  c1.SaveAs(epsFile.c_str());

  // ROOT can not produce gif file in the batch mode
  // run pstopnm and ppmtogif to produce gif file

  string pwd = gSystem->WorkingDirectory();
  gSystem->cd(m_dir.c_str());

  string cmd1("pstopnm -ppm -xborder 0 -yborder 0 -portrait -nocrop -xsize ");
  cmd1 += ToString(att.m_xPixel);
  cmd1 += " -ysize ";
  cmd1 += ToString(att.m_yPixel);
  cmd1 += " ";
  epsFile = att.m_file;
  epsFile += ".eps";
  cmd1 += epsFile;
  gSystem->Exec(cmd1.c_str());

  string ppmFile(epsFile);
  ppmFile += "001.ppm";
  string gifFile(att.m_file);
  gifFile += ".gif";

  string cmd2("ppmtogif ");
  cmd2 += ppmFile;
  cmd2 += " > ";
  cmd2 += gifFile;
  gSystem->Exec(cmd2.c_str());

  gSystem->cd(pwd.c_str());
}

void TestReport::producePlot(TObject** h, const PlotAttribute& att, int nRow, 
			     int nCol)
{
  TCanvas c1("c1", "produce eps file");
  gPad->SetBorderMode(0);
  gPad->SetFillColor(10);
  c1.Divide(nCol, nRow, 0.001, 0.001);

  for(int i = 0; i != nRow*nCol; ++i) {

    //    gStyle->SetTitleH(0.12);
    //    gStyle->SetTitleW(0.2);
    //    gStyle->SetTitleFontSize(4);
    c1.cd(i+1);
    gStyle->SetStatH(0.4);
    gPad->SetBottomMargin(0.05*nRow);

    if(att.m_yLog) gPad->SetLogy();
    if(att.m_zLog) {
      cout << "set logz" << endl;
      gPad->SetLogz();
    }

    if(TGraph* gr = dynamic_cast<TGraph*>(h[i])) {
      gr->Draw("A*");
    }
    else if(TH2F* h2 = dynamic_cast<TH2F*>(h[i])) { 
      h2->Draw("LEGO");
    }
    else if(TH1F* h1 = dynamic_cast<TH1F*>(h[i])){
      gStyle->SetOptStat(att.m_statMode);
      h1->Draw();
    }
  }

  string epsFile(m_dir);
  epsFile += "/";
  epsFile += att.m_file;
  epsFile += ".eps";
  c1.SaveAs(epsFile.c_str());

  // ROOT can not produce gif file in the batch mode
  // run pstopnm and ppmtogif to produce gif file

  string pwd = gSystem->WorkingDirectory();
  gSystem->cd(m_dir.c_str());

  string cmd1("pstopnm -ppm -xborder 0 -yborder 0 -portrait -nocrop -xsize ");
  cmd1 += ToString(att.m_xPixel);
  cmd1 += " -ysize ";
  cmd1 += ToString(att.m_yPixel);
  cmd1 += " ";
  epsFile = att.m_file;
  epsFile += ".eps";
  cmd1 += epsFile;
  gSystem->Exec(cmd1.c_str());

  string ppmFile(epsFile);
  ppmFile += "001.ppm";
  string gifFile(att.m_file);
  gifFile += ".gif";

  string cmd2("ppmtogif ");
  cmd2 += ppmFile;
  cmd2 += " > ";
  cmd2 += gifFile;
  gSystem->Exec(cmd2.c_str());

  gSystem->cd(pwd.c_str());
}

void TestReport::insertPlot(const PlotAttribute& att)
{
  string gifFile(att.m_file);
  gifFile += ".gif";

  *(m_report) << "@htmlonly <div align=\"center\"> <p>"
	      << boldFaceHtml(eraseLatexStr(att.m_caption)) 
	      << "</p> <img src=\"../" 
	      << gifFile << "\" alt=\"../" << gifFile 
	      << "\"> </div> @endhtmlonly" << endl;
 
  string epsFile(att.m_file);
  epsFile += ".eps";

  string cap =  boldFaceLatex(eraseHtmlStr(att.m_caption));
  *(m_report) << "@latexonly \\begin{figure}[H]" << endl
	      << "\\begin{center}" << endl
	      << "\\includegraphics[height=" << att.m_height << "cm,width="
	      << att.m_width << "cm]{" << epsFile << "}\\caption{"
	      << cap << "}" << endl
	      << "\\end{center}" << endl
	      << "\\end{figure}" << endl << "@endlatexonly" << endl;
}

void TestReport::applyDash(string* x, int n) const 
{
  for(int i = 0; i != n; ++i) {

    string s;
    for(const char* p = x[i].c_str(); *p != 0; ++p) {
      if(*p == '_') s += "\\";
      if(*p == '%') s += "\\";
      s += *p;
    }

    x[i] = s;
  }
}      

void TestReport::printHtmlTable(const TableDef& r)
{
  *(m_report) << endl << "@htmlonly" << endl;
  *(m_report) << "<table border=\"1\" width=\"100%\">" << endl;
  *(m_report) << "<caption> " << r.m_caption << " </caption>" << endl;

  for(int i = 0; i != r.m_nRow; ++i) {
    *(m_report) << "  <tr>" << endl;
    for(int j = 0; j != r.m_nCol; ++j) {
      *(m_report) << "    <td> " << r.m_table[i*r.m_nCol+j] 
		  << " </td>" << endl;
    }
    *(m_report) << "  </tr>" << endl;
  }

  *(m_report) << "</table>" << endl;
  *(m_report) << "@endhtmlonly" << endl << endl;
}

void TestReport::printLatexTable(const TableDef& r)
{
  *(m_report) << endl << "@latexonly" << endl;
  *(m_report) << "\\begin{table}[h]" << endl;
  *(m_report) << "\\begin{center}" << endl;
  *(m_report) << "\\caption{" << r.m_caption << "}" << endl;
  *(m_report) << "\\label{" << r.m_label << "}" << endl;

  *(m_report) << "\\begin{tabular}{";
  for(int i = 0; i != r.m_nCol; ++i) {
    *(m_report) << "|c";
  }
  *(m_report) << "|} \\hline" << endl;

  for(int i = 0; i != r.m_nRow; ++i) {
    for(int j = 0; j != r.m_nCol-1; ++j) {
      *(m_report) << r.m_table[i*r.m_nCol+j] << " & ";
    }
    *(m_report) << r.m_table[(i+1)*r.m_nCol-1] << " \\\\ \\hline" << endl;
  }

  *(m_report) << "\\end{tabular}" << endl;
  *(m_report) << "\\end{center}" << endl;
  *(m_report) << "\\end{table}" << endl;
  *(m_report) << "@endlatexonly" << endl << endl;
}

void TestReport::printGltTriggerTable()
{
  int nRow = 3;
  int nCol = enums::number_of_trigger_bits+1;
  string table[nRow][nCol];

  table[0][0] = "Trigger bit";
  for(int i = 1; i != nCol; ++i) {
    table[0][i] = ToString(i-1);
  }
 
  table[1][0] = "Trigger";
  table[1][1] = "ACD Low";
  table[1][2] = "TKR";
  table[1][3] = "CAL Low";
  table[1][4] = "CAL High";
  table[1][5] = "ACD High";
  table[1][6] = "THROTTLE";
  
  table[2][0] = "Number of events";
  for(int i = 1; i != enums::number_of_trigger_bits+1; ++i) {
    table[2][i] = ToString(m_nEvtGltTrigger[i-1]);
  }

  TableDef t((string*) table, "Trigger bits used in TriggerAlg", "gltTriggerTable", nRow, nCol);

  printHtmlTable(t);
  printLatexTable(t);
}

void TestReport::printCondSummaryTable()
{
  int nRow = 3;
  int nCol = enums::GEM_offset+1;
  string table[nRow][nCol];

  table[0][0] = "Trigger bit";
  for(int i = 1; i != nCol; ++i) {
    table[0][i] = ToString(i-1);
  }

  table[1][0] = "Summary";
  table[1][1] = "ROI";
  table[1][2] = "TKR";
  table[1][3] = "CAL Low";
  table[1][4] = "CAL High";
  table[1][5] = "CNO";
  table[1][6] = "Periodic";
  table[1][7] = "Solicited";
  table[1][8] = "External";

  table[2][0] = "Number of events";
  for(int i = 1; i != enums::GEM_offset+1; ++i) {
    table[2][i] = ToString(m_nEvtGemTrigger[i-1]);
  }

  TableDef t((string*) table, "GEM Condition summary word", "condSummaryTable", nRow, nCol);

  printHtmlTable(t);
  printLatexTable(t);
}

void TestReport::printNTrackTable()
{
  TableDef d;

  string caption("Ratio of events with different number of reconstructed TKR tracks.");
  d.m_caption = caption.c_str();
  d.m_label = "nEventDigi";
  d.m_nRow = 2;
  d.m_nCol = 5;

  string table[d.m_nRow][d.m_nCol];
  d.m_table = (string*) table;
  table[0][1] = "0 tracks"; 
  table[0][2] = "1 track"; 
  table[0][3] = "2 tracks";
  table[0][4] = "More than 2 tracks";

  table[1][0] = "Ratio";
  table[1][1] = ToString(m_nTkrTrack->GetBinContent(1)/double(m_nEvent));
  table[1][2] = ToString(m_nTkrTrack->GetBinContent(2)/double(m_nEvent));
  table[1][3] = ToString(m_nTkrTrack->GetBinContent(3)/double(m_nEvent));
  int n = m_nEvent - m_nTkrTrack->GetBinContent(1) - m_nTkrTrack->GetBinContent(2) - m_nTkrTrack->GetBinContent(3);
  table[1][4] = ToString(n/double(m_nEvent));

  printHtmlTable(d);

  printLatexTable(d);
}

void TestReport::printNDigiTable()
{
  if(m_nTkrTrigger == 0) {
    *m_report << "There are no events with TKR trigger from GEM condition summary word. Please check the config report to see whether the GEM is present." << endl;
    return;
  }

  TableDef d;

  string caption("Number of events with different number of TKR digis. There are ");
  caption += ToString(m_nTkrTrigger); 
  caption += " events with 3 in a row TKR trigger in the GEM.";
  d.m_caption = caption.c_str();
  d.m_label = "nEventDigi";

  d.m_nRow = 8;
  d.m_nCol = 3;
  string table[d.m_nRow][d.m_nCol];
  d.m_table = (string*) table;
  table[0][0] = "Number of TKR digis";
  table[0][1] = "Number of TKR events"; 
  table[0][2] = "Ratio";

  for(int i = 1; i != d.m_nRow; ++i) {
    if(i == 7) {
      table[i][0] =  ">= 6";
    }
    else {
      table[i][0] = ToString(i-1);
    }
    table[i][1] = ToString(m_nEventDigi[i-1]);  
    table[i][2] = ToString(float(m_nEventDigi[i-1])/m_nTkrTrigger);
  }

  printHtmlTable(d);

  table[7][0] =  "$\\geq$ 6";
  applyDash((string*) table, d.m_nRow*d.m_nCol);
  printLatexTable(d);
}

void TestReport::printNDigiTowerTable()
{
  if(m_nTkrTrigger == 0) {
    //    *m_report << "There are no events with TKR trigger." << endl;
    return;
  }

  double tower[g_nTower], bad[g_nTower];

  for(int i = 0; i != g_nTower; ++i) {
    tower[i] = i;
    if(m_nTkrEvent[i] != 0) {
      bad[i] = float(m_nTkrBadEvent[i])/m_nTkrEvent[i];
    }
    else {
      bad[i] = 0.;
    }
  }

  int nRow = 2;
  int nCol = 17;
  string table[nRow][nCol];

  table[0][0] = "Tower";
  table[1][0] = "Ratio";

  for(int i = 1; i != nCol; ++i) {
    table[0][i] = ToString(tower[i-1]);
    table[1][i] = ToString(bad[i-1]);
  }

  TableDef t((string*) table, "Ratio of events with a TKR trigger and less than 6 digis in each tower", "nDigiTowerTable", nRow, nCol);

  printHtmlTable(t);
  printLatexTable(t);
}

void TestReport::produceNDigiPlot()
{
  double tower[g_nTower], bad[g_nTower];

  // no. of towers with TKR trigger
  int nDataTower = 0;

  for(int i = 0; i != g_nTower; ++i) {
    if(m_nTkrEvent[i] != 0) {
      bad[nDataTower] = float(m_nTkrBadEvent[i])/m_nTkrEvent[i];
      tower[nDataTower] = i;
      ++nDataTower;
    }
  }

  m_nDigi = new TGraph(nDataTower, tower, bad);

  TCanvas temp("temp", "temp");
  m_nDigi->Draw("A*");
  m_nDigi->GetXaxis()->SetTitle("Tower");
  m_nDigi->GetYaxis()->SetTitle("Ratio of events with less than 6 digis");

  string file(m_prefix);
  file += "_nDigi";
  PlotAttribute att(file.c_str(), "Ratio of events with TKR trigger and at least 6 digis", "nDigi", 0);
  producePlot(m_nDigi, att);
  insertPlot(att);
}

void TestReport::produceAveTotPlots()
{
  // *2 because there are 2 TOTs per plane
  int nTot = g_nPlane * 2;
  float id[nTot], ave[nTot];

  TGraph* graphs[g_nTower];

  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nPlane; ++j) {
      for(int k = 0; k != 2; ++k) {

	int iTot = j*2 + k;
	ave[iTot] = m_tot[i][j][k]->GetMean();
	id[iTot] = float(iTot)/2.;

      }
    }
    graphs[i] = new TGraph(nTot, id, ave);
    TCanvas temp("temp", "temp");
    graphs[i]->Draw("A*");
    char name[] = "Layer(tower00)";
    sprintf(name, "Layer(tower%02d)", i);
    GraphAttribute att(name, "Ave TOT value", 0.1, 0.07, 0.6, 0.4, 0.3);
    setGraphParameters(graphs[i], att);
  }

  string file(m_prefix);
  file += "_tot0";
  PlotAttribute att(file.c_str(), "Average TOT value in tower 0 to tower 7(excluding 0 and saturation). Plane 0 is at bottom of the tracker.", "tot0", 0, 0, 18, 18, 909, 615);
  producePlot((TObject**) graphs, att, 4, 2);
  insertPlot(att);

  file = m_prefix;
  file += "_tot1";
  att.set(file.c_str(), "Average TOT value in tower 8 to tower 15(excluding 0 and saturation). Plane 0 is at bottom of the tracker.", "tot1", 0, 0, 18, 12.2, 909, 615);
  att.m_2dPlotType = PlotAttribute::COLZ;
  producePlot((TObject**) graphs+8, att, 4, 2);
  insertPlot(att);
}

void TestReport::produceNhits2DPlot()
{
  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nPlane; ++j) {

      if(m_nEvtPlane[i][j]) {
	m_nHit2D->Fill(j, i, double(m_nHitPlane[i][j])/m_nEvtPlane[i][j]);
      }
      m_nZeroHit2D->Fill(j+0.25, i, double(m_nEvent-m_nEvtHitPlane[i][j][0])/m_nEvent);
      m_nZeroHit2D->Fill(j+0.75, i, double(m_nEvent-m_nEvtHitPlane[i][j][1])/m_nEvent);
    }
  }

  string file(m_prefix);
  file += "_nhits2d";
  PlotAttribute att(file.c_str(), "Average number of strip hits in a TKR plane. Events with zero hits are not used in calculating the average", "nhits2d");
  producePlot(m_nHit2D, att);
  insertPlot(att);

  file = m_prefix;
  file += "_nZeroHit2d";
  att.set(file.c_str(), "Fraction of events with 0 strip hits. Note that the plot contains information from both ends of the same plane. For example, the bin at 0-0.5 along the X axis corresponds to the value from the GTRC close to strip 0 in plane 0, the bin at 0.5-1. along the X axis corresponds to the value from the GTRC close to strip 1536 in plane 0", "nZeroHit2d");
  producePlot(m_nZeroHit2D, att);
  insertPlot(att);
}

void TestReport::produceCalNhits2DPlot()
{
  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nCalLayer; ++j) {
      if(m_nEvtCalHit[i][j]) {
	if(i==4) {
	  cout << "i = " << i << " " << m_nCalHit[i][j] << " " << m_nEvtCalHit[i][j];
	}
	m_nCalHit2D->Fill(j, i, double(m_nCalHit[i][j])/m_nEvtCalHit[i][j]);
      }
      if(m_nEventNoPeriodic > m_nEvtCalHit[i][j]) {
	//m_nZeroCalHit2D->Fill(j, i, double(m_nEvent-m_nEvtCalHit[i][j])/m_nEvent);
        m_nZeroCalHit2D->Fill(j, i, double(m_nEventNoPeriodic-m_nEvtCalHit[i][j])/m_nEventNoPeriodic);
      }
    }
  }

  string file(m_prefix);
  file += "_nCalHits2d";
  PlotAttribute att(file.c_str(), "Average number of crystal hits in a particular CAL layer. Note 0 hit is not used to calculate the average. Periodic triggers have been taken out.", "nCalHits2d");
  producePlot(m_nCalHit2D, att);
  insertPlot(att);

  file = m_prefix;
  file += "_nZeroCalHit2d";
  att.set(file.c_str(), "Fraction of events with zero hits in a particular CAL layer. Periodic triggers have been taken out.", "nZeroCalHit2d");
  producePlot(m_nZeroCalHit2D, att);
  insertPlot(att);
}

void TestReport::produceZeroSatTot2DPlot()
{
  /*
  string file = m_prefix;
  file += "_totZero";
  PlotAttribute att(file.c_str(), "Fraction of events with TOT equal to zero in each plane (plane 0 is at the bottom) in each tower. Note the plot contains info for both tot counters in the same plane. For example, bin at 0-0.5 along the X axis corresponds to tot value from GTRC close to strip 0 at plane 0, bin at 0.5-1. along the X axis corresponds to tot value from GTRC close to strip 1536 at plane 0", "satTot");
  att.m_nColor = 4;
  producePlot(m_totZero2D, att);
  insertPlot(att);
  */

  // scale the histogram
  int nEvts[g_nPlane*g_nEnd][g_nTower];

  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nPlane; ++j) {
      nEvts[j*2][i] = m_nEvtHitPlane[i][j][0];
      nEvts[j*2+1][i] = m_nEvtHitPlane[i][j][1];
    }
  }

  scale2DHist(m_totSat2D, (int*) nEvts);

  string file = m_prefix;
  file += "_totSat";
  PlotAttribute att(file.c_str(), "Fraction of events with saturated TOT value of 250 counts (1 count = 200 ns, plane 0 is at the bottom). When calculating the fraction, the denominator is the number of events with at least 1 hit in the corresponding region. The plot contains information for both TOT counters in the same plane. For example, the bin at tower 0, plane 0-0.5 corresponds to the TOT value from the GTRC close to strip 0 in plane 0 in tower 0. The bin at tower 0, plane 0.5-1. corresponds to the TOT value from the GTRC close to strip 1536 in plane 0 in tower 0", "satTot");
  producePlot(m_totSat2D, att);
  insertPlot(att);
}

void TestReport::produceTotNoise2DPlot()
{
  // scale the histogram
  int nEvts[g_nPlane*g_nEnd][g_nTower];

  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nPlane; ++j) {
      nEvts[j*2][i] = m_nEvtHitPlane[i][j][0];
      nEvts[j*2+1][i] = m_nEvtHitPlane[i][j][1];
    }
  }

  scale2DHist(m_totNoise2D, (int*) nEvts);

  string file = m_prefix;
  file += "_totNoise";
  PlotAttribute att(file.c_str(), "Fraction of events with TOT equal to zero but nonzero number of strip hits (plane 0 is at the bottom). When calculating the fraction, the denominator is the number of events with at least 1 hit in the corresponding region. The plot contains information for both TOT counters in the same plane (one for each end). For example, the bin at 0-0.5 along the X axis corresponds to the TOT value from the GTRC close to strip 0 in plane 0. The bin at 0.5-1. along the X axis corresponds to the TOT value from the GTRC close to strip 1535 in plane 0", "TotNoise");
  producePlot(m_totNoise2D, att);
  insertPlot(att);
}

void TestReport::produceAveTot2DPlot()
{
  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nPlane; ++j) {
      m_totAve2D->Fill(j+0.25, i, m_tot[i][j][0]->GetMean());
      m_totAve2D->Fill(j+0.75, i, m_tot[i][j][1]->GetMean());
    }
  }

  string file(m_prefix);
  file += "_totAve2D";
  PlotAttribute att(file.c_str(), "Average TOT value(excluding 0 and saturation value, plane 0 is at bottom). Note the plot contains info for both tot counters in the same plane. For example, bin at tower 0, plane 0-0.5 corresponds to tot value from GTRC close to strip 0 at plane 0 in tower 0. Bin at tower 0, plane 0.5-1. corresponds to tot value from GTRC close to strip 1536 at plane 0 in tower 0", "totAve2D");
  producePlot(m_totAve2D, att);
  insertPlot(att);
}

void TestReport::produceReconDirPlots()
{
  string file(m_prefix);
  file += "_reconDirXY";
  PlotAttribute att(file.c_str(), "Reconstructed event direction along the X and the Y axis. The color scheme represents the number of events in a bin. The values are obtained from the first reconstructed vertex.", "reconDirXY");
  producePlot(m_reconDirXY, att);
  insertPlot(att);

  file = m_prefix;
  file += "_reconDirZ";
  att.set(file.c_str(), "Reconstructed event direction along the Z axis.The value is obtained from the first reconstructed vertex.", "reconDirZ");
  producePlot(m_reconDirZ, att);
  insertPlot(att);
}

void TestReport::produceReconPosPlots()
{
  string file(m_prefix);
  file += "_reconPosXY";
  PlotAttribute att(file.c_str(), "Reconstructed event position along the X and the Y axis. The color scheme represents the number of events in a bin. The values are obtained from the first reconstructed vertex.", "reconPosXY");
  att.m_statMode = 110011;
  att.m_statBox = 1;
  producePlot(m_reconPosXY, att);
  insertPlot(att);

  file = m_prefix;
  file += "_reconPosZ";
  att.set(file.c_str(), "Reconstructed event position along the Z axis. The value is obtained from the first reconstructed vertex.", "reconPosZ", 1);
  att.m_statMode = 111111;
  producePlot(m_reconPosZ, att);
  insertPlot(att);
}

void TestReport::produceReconEnePlots()
{
  string file(m_prefix);
  file += "_reconEne";
  PlotAttribute att(file.c_str(), "Reconstructed event energy (MeV) assigned to the first reconstructed vertex. By default, it is calculated based on the number of hits in the tracker and the amount of energy measured in the calorimeter.", "reconEne", 1);
  producePlot(m_reconEne, att);
  insertPlot(att);
}

void TestReport::produceCalEneSum2DPlot()
{
  string file = m_prefix;
  file += "_calSumEne";
  PlotAttribute att(file.c_str(), "Sum of crystal energies in the CAL cluster (MeV). Note that there is only one CAL cluster produced by CalRecon at the moment. Energy measured in each crystal is obtained by using the getEnergy() member function of the CalXtalRecData class.", "calSumEne", 1);
  att.m_statMode = 111111;
  producePlot(m_calSumEne, att);
  insertPlot(att);
}

void TestReport::produceCalEneLayer2DPlot()
{
  for(int i = 0; i != g_nTower; ++i) {
    for(int j = 0; j != g_nCalLayer; ++j) {
      if(m_nCalEneLayer[i][j]) {
	m_calEneLayer2D->Fill(j, i, m_calEneLayer[i][j]/m_nCalEneLayer[i][j]);
      }
      if(m_nEvent > m_nCalEneLayer[i][j]) {
	m_zeroCalEneLayer2D->Fill(j, i, double(m_nEvent-m_nCalEneLayer[i][j])/m_nEvent);
      }
    }
  }

  string file(m_prefix);
  file += "_calEneLayer2d";
  PlotAttribute att(file.c_str(), "Average energies (MeV) measured in each CAL layer, scaled by the color Z axis. Note that zero energy is not used in calculating the average. Energy measured in each crystal is obtained by using the getEnergy() member function of the CalXtalRecData class.", "calEneLayer2d");
  producePlot(m_calEneLayer2D, att);
  insertPlot(att);

  file = m_prefix;
  file += "_zeroCalEndLayer2d";
  att.set(file.c_str(), "Fraction of events with zero energy measured for all CAL layers.", "zeroCalEndLayer2d");
  producePlot(m_zeroCalEneLayer2D, att);
  insertPlot(att);
}

void TestReport::scale2DHist(TH2F* h, int* nEvents)
{
  int nBinsX = h->GetNbinsX();
  int nBinsY = h->GetNbinsY();

  for(int i = 0; i != nBinsX; ++i) {
    for(int j = 0; j != nBinsY; ++j) {

      // note in ROOT, first bin is bin 1, bin 0 contains underflow

      int content = (int) h->GetBinContent(i+1, j+1);
      if(content) {
	h->SetBinContent(i+1, j+1, double(content)/ (*(nEvents+nBinsY*i+j)) );
      }
    }
  }
}


void TestReport::produceTriggerRatePlot()
{
  string file(m_prefix);
  file += "_triggerRate";
  PlotAttribute att(file.c_str(), "Trigger rates for 30 time intervals","triggerRate");
  att.m_statMode = 11;
  producePlot(m_triggerRate, att);
  insertPlot(att);

  file = m_prefix;
  file += "_triggerLivetimeRate";
  att.set(file.c_str(), "Livetime corrected trigger rates for 30 time intervals","triggerLivetimeRate");
  att.m_statMode = 11;
  producePlot(m_triggerLivetimeRate, att);
  insertPlot(att);

  file = m_prefix;
  file += "_livetimeRate";
  att.set(file.c_str(), "Livetime in percent for 30 time intervals","livetimeRate");
  att.m_statMode = 11;
  producePlot(m_livetimeRate, att);
  insertPlot(att);

  file = m_prefix;
  file += "_deadzoneRate";
  att.set(file.c_str(), "Deadzone rates for 30 time intervals","deadzoneRate");
  att.m_statMode = 11;
  producePlot(m_deadzoneRate, att);
  insertPlot(att);

  file = m_prefix;
  file += "_discardedRate";
  att.set(file.c_str(), "Discarded rates for 30 time intervals","discardedRate");
  att.m_statMode = 11;
  producePlot(m_discardedRate, att);
  insertPlot(att);
}


void TestReport::produceTriggerPerTowerPlot()
{
  string file(m_prefix);
  file += "_tkrPerTower";
  PlotAttribute att(file.c_str(), "Number of TKR triggers per tower","tkrPerTower");
  att.m_statMode = 11;
  producePlot(m_tkrPerTower, att);
  insertPlot(att);

  file = m_prefix;
  file += "_calLoPerTower";
  att.set(file.c_str(), "Number of CAL LO triggers per tower","calLoPerTower");
  att.m_statMode = 11;
  producePlot(m_calLoPerTower, att);
  insertPlot(att);

  file = m_prefix;
  file += "_calHiPerTower";
  att.set(file.c_str(), "Number of CAL Hi triggers per tower","calHiPerTower");
  att.m_statMode = 11;
  producePlot(m_calHiPerTower, att);
  insertPlot(att);
}


void TestReport::produceCondArrivalTimesPlots()
{

  string file(m_prefix);
  file += "_condArrivalTimesTKR";
  PlotAttribute att(file.c_str(), "Condition arrival times for TKR triggers. Periodic triggers have been taken out. ","condArrivalTimesTKR",true);
  producePlot(m_condArrivalTimeTKR, att);
  insertPlot(att);

  file = m_prefix;
  file += "_condArrivalTimesROI";
  att.set(file.c_str(), "Condition arrival times for ROI triggers. Periodic triggers have been taken out. If the ROI is not allowed to open the trigger window about half will be at 31.","condArrivalTimesROI",true);
  producePlot(m_condArrivalTimeROI, att);
  insertPlot(att);

  file = m_prefix;
  file += "_condArrivalTimesCalLo";
  att.set(file.c_str(), "Condition arrival times for CAL LO triggers. Periodic triggers have been taken out. ","condArrivalTimesCalLo",true);
  producePlot(m_condArrivalTimeCalLo, att);
  insertPlot(att);

  file = m_prefix;
  file += "_condArrivalTimesCalHi";
  att.set(file.c_str(), "Condition arrival times for CAL HI triggers. Periodic triggers have been taken out. ","condArrivalTimesCalHi",true);
  producePlot(m_condArrivalTimeCalHi, att);
  insertPlot(att);

  file = m_prefix;
  file += "_condArrivalTimesCNO";
  att.set(file.c_str(), "Condition arrival times for CNO triggers. Periodic triggers have been taken out. ","condArrivalTimesCNO",true);
  producePlot(m_condArrivalTimeCNO, att);
  insertPlot(att);
}


void TestReport::produceAcdTriggerPlots()
{

  string file(m_prefix);
  file += "_acdGemVeto";
  PlotAttribute att(file.c_str(), "Number of Acd Gem vetos by tile.  Expect to see spikes at 15,31,47,63.  These are the large tiles at the bottom of the sides of the ACD.","AcdVetoPerTile");
  att.m_statMode = 11;
  producePlot(m_AcdGemVetoMap, att);
  insertPlot(att);

  file = m_prefix;
  file += "_acdRoiVeto";
  att.set(file.c_str(), "Number of Acd ROI by tower,  This bit is set if any of the tile in the ROI fired a veto.  This bit is independent of if the Tower bit is set.","AcdRoiPerTower");
  att.m_statMode = 11;
  m_AcdGemRoiMap->SetMinimum(0.);
  producePlot(m_AcdGemRoiMap, att);
  insertPlot(att);

  file = m_prefix;
  file += "_acdCnoVeto";
  att.set(file.c_str(), "Number of Acd Cno by board.  This bit is the OR of all the CNO signals on a single board. Because of the tile to board mapping, a factor two variation between boards is normal.","AcdCnoPerBoard");
  att.m_statMode = 11;
  m_AcdGemCnoMap->SetMinimum(0.);
  producePlot(m_AcdGemCnoMap, att);
  insertPlot(att);
}



void TestReport::produceEpuPlot()
{
  string file(m_prefix);
  file += "_epu";
  PlotAttribute att(file.c_str(), "Crate number","epu",true);
  att.m_statMode = 11;
  producePlot(m_epu, att);
  insertPlot(att);

  if (m_nbrEventsDataGramsEpu0 > 0 ) {
    file = m_prefix;
    file += "_datagramsEPU0";
    att.set(file.c_str(), "Number of events per datagram - EPU0","datagramsEPU0",true);
    producePlot(m_datagramsEPU0, att);
    insertPlot(att);
  }
  if (m_nbrEventsDataGramsEpu1 > 0 ) {
    file = m_prefix;
    file += "_datagramsEPU1";
    att.set(file.c_str(), "Number of events per datagram - EPU1","datagramsEPU1",true);
    producePlot(m_datagramsEPU1, att);
    insertPlot(att);
  }
  if (m_nbrEventsDataGramsEpu2 > 0 ) {
    file = m_prefix;
    file += "_datagramsEPU2";
    att.set(file.c_str(), "Number of events per datagram - EPU2","datagramsEPU2",true);
    producePlot(m_datagramsEPU2, att);
    insertPlot(att);
  }
  if (m_nbrEventsDataGramsSiu0 > 0 ) {
    file = m_prefix;
    file += "_datagramsSIU0";
    att.set(file.c_str(), "Number of events per datagram - SIU0","datagramsSIU0",true);
    producePlot(m_datagramsSIU0, att);
    insertPlot(att);
  }
  if (m_nbrEventsDataGramsSiu1 > 0 ) {
    file = m_prefix;
    file += "_datagramsSiu1";
    att.set(file.c_str(), "Number of events per datagram - SIU1","datagramsSIU1",true);
    producePlot(m_datagramsSIU1, att);
    insertPlot(att);
  }


  if (m_isLATTE != 1) {
    if (m_nbrEventsDataGramsEpu0 > 0 ) {
      file = m_prefix;
      file += "_deltaTimeDGCTEvtEPU0";
      att.set(file.c_str(), "CCSDS Secondary Header Time minus the event time for events from EPU0 [seconds]","deltaTimeDGCTEvtEPU0",true);
      att.m_statMode = 1111;
      producePlot(m_deltaTimeDGCTEvtEPU0, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsEpu1 > 0 ) {
      file = m_prefix;
      file += "_deltaTimeDGCTEvtEPU1";
      att.set(file.c_str(), "CCSDS Secondary Header Time minus the event time for events from EPU1 [seconds]","deltaTimeDGCTEvtEPU1",true);
      att.m_statMode = 1111;
      producePlot(m_deltaTimeDGCTEvtEPU1, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsEpu2 > 0 ) {
      file = m_prefix;
      file += "_deltaTimeDGCTEvtEPU2";
      att.set(file.c_str(), "CCSDS Secondary Header Time minus the event time for events from EPU2 [seconds]","deltaTimeDGCTEvtEPU2",true);
      att.m_statMode = 1111;
      producePlot(m_deltaTimeDGCTEvtEPU2, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsSiu0 > 0 ) {
      file = m_prefix;
      file += "_deltaTimeDGCTEvtSIU0";
      att.set(file.c_str(), "CCSDS Secondary Header Time minus the event time for events from SIU0 [seconds]","deltaTimeDGCTEvtSIU0",true);
      att.m_statMode = 1111;
      producePlot(m_deltaTimeDGCTEvtSIU0, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsSiu1 > 0 ) {
      file = m_prefix;
      file += "_deltaTimeDGCTEvtSIU1";
      att.set(file.c_str(), "CCSDS Secondary Header Time minus the event time for events from SIU1 [seconds]","deltaTimeDGCTEvtESIU1",true);
      att.m_statMode = 1111;
      producePlot(m_deltaTimeDGCTEvtSIU1, att);
      insertPlot(att);
    }

    if (m_nbrEventsDataGramsEpu0 > 0 ) {
      file = m_prefix;
      file += "_deltaEventIDEPU0";
      att.set(file.c_str(), "Delta Event ID For Successive Events From EPU0","deltaEventIDEPU0",true);
      att.m_statMode = 1111;
      producePlot(m_deltaEventIDEPU0, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsEpu1 > 0 ) {
      file = m_prefix;
      file += "_deltaEventIDEPU1";
      att.set(file.c_str(), "Delta Event ID For Successive Events From EPU1","deltaEventIDEPU1",true);
      att.m_statMode = 1111;
      producePlot(m_deltaEventIDEPU1, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsEpu2 > 0 ) {
      file = m_prefix;
      file += "_deltaEventIDEPU2";
      att.set(file.c_str(), "Delta Event ID For Successive Events From EPU2","deltaEventIDEPU2",true);
      att.m_statMode = 1111;
      producePlot(m_deltaEventIDEPU2, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsSiu0 > 0 ) {
      file = m_prefix;
      file += "_deltaEventIDSIU0";
      att.set(file.c_str(), "Delta Event ID For Successive Events From SIU0","deltaEventIDSIU0",true);
      att.m_statMode = 1111;
      producePlot(m_deltaEventIDSIU0, att);
      insertPlot(att);
    }
    if (m_nbrEventsDataGramsSiu1 > 0 ) {
      file = m_prefix;
      file += "_deltaEventIDSIU1";
      att.set(file.c_str(), "Delta Event ID For Successive Events From SIU1","deltaEventIDSIU1",true);
      att.m_statMode = 1111;
      producePlot(m_deltaEventIDSIU1, att);
      insertPlot(att);
    }

  }
}

void TestReport::produceGemDiscardedPlot()
{
  string file(m_prefix);
  file += "_gemDiscarded";
  PlotAttribute att(file.c_str(), "Number of GEM discarded events between two successive read out events.","gemDiscarded",true);
  producePlot(m_gemDiscarded, att);
  insertPlot(att);

  file = m_prefix;
  file += "_gemDiscardedTime";
  att.set(file.c_str(), "Time between the previous read out event and the last discarded event in system clock ticks (nominally 50 ns). Only non-saturated values of the GEM time counters have been used.","gemDiscardedTime",true);
  producePlot(m_gemDiscardedTime, att);
  insertPlot(att);

  file = m_prefix;
  file += "_gemDeadzone";
  att.set(file.c_str(), "Number of GEM deadzone events between two successive read out events.","gemDeadzone",true);
  producePlot(m_gemDeadzone, att);
  insertPlot(att);
}


void TestReport::produceTimeIntervalPlotSBC()
{
  string file(m_prefix);
  file += "_timeInterval";
  PlotAttribute att(file.c_str(), "Time interval between adjacent events in millseconds. Note that this interval is between the time when the events are built, NOT the GEM trigger time. The time is measured by the 16MHZ clock in the power PC.", "timeInterval", true);
  producePlot(m_timeInterval, att);
  insertPlot(att);

  file = m_prefix;
  file += "_timeIntervalCut";
  att.set(file.c_str(), "Time interval between adjacent events in millseconds with a cut of 1 ms. Note that this interval is between the time when the events are built, NOT the GEM trigger time. The time is measured by the 16MHZ clock in the power PC.", "timeIntervalCut", true);
  producePlot(m_timeIntervalCut, att);
  insertPlot(att);
}



//
void TestReport::produceTimeIntervalPlotGEM()
{
  string file(m_prefix);
  file = m_prefix;
  file += "_timeIntervalGem";
  PlotAttribute att(file.c_str(), "Time interval between adjacent events in milliseconds. Note that this interval is measured in the GEM. The time is stored in a 16 bit counter, each count is nominally 50 ns, so the time will saturate at roughly 3.3 ms.", "timeIntervalGem", true);
  producePlot(m_timeIntervalGem, att);
  insertPlot(att);

  file = m_prefix;
  file += "_timeIntervalGemZoom";
  att.set(file.c_str(), "Time interval between adjacent events as measured by the GEM in system clock ticks with a cut of 1500 ticks. The time is stored in a 16 bit counter, each count is nominally 50 ns.", "timeIntervalGemZoom", true);
  producePlot(m_timeIntervalGemZoom, att);
  insertPlot(att);

  file = m_prefix;
  file += "_timeIntervalElapsed";
  att.set(file.c_str(), "Elapsed time between adjacent events in milliseconds. This is not corrected for any drift in the clock. It assumes a nominal system clock of 50 ns.", "timeIntervalElapsed", true);
  producePlot(m_timeIntervalElapsed, att);
  insertPlot(att);


  file = m_prefix;
  file += "_deltaWindowOpenTime";
  att.set(file.c_str(), "Delta window open time in milliseconds. Note that this interval is measured in the GEM. The time is stored in a 16 bit counter, each count is nominally 50 ns.", "deltaWindowOpenTime", true);
  producePlot(m_deltaWindowOpenTime, att);
  insertPlot(att);

  file = m_prefix;
  file += "_deltaWindowOpenTimeZoom";
  att.set(file.c_str(), "Delta window open time as measured by the GEM in system clock ticks with a cut of 1500 ticks. The time is stored in a 16 bit counter, each count is nominally 50 ns.", "deltaWindowOpenTimeZoom", true);
  producePlot(m_deltaWindowOpenTimeZoom, att);
  insertPlot(att);

  file = m_prefix;
  file += "_tick20MHzDeviation";
  att.set(file.c_str(), "Number of ticks between successive 1-PPS - Deviation from 20 MHz.", "tick20MHzDeviation", true);
  producePlot(m_tick20MHzDeviation, att);
  insertPlot(att);

  file = m_prefix;
  file += "_tick20MHzDeviationZoom";
  att.set(file.c_str(), "Number of ticks between successive 1-PPS - Deviation from 20 MHz - Zoom.", "tick20MHzDeviationZoom", true);
  producePlot(m_tick20MHzDeviationZoom, att);
  insertPlot(att);
}





string TestReport::boldFaceLatex(const string& s) 
{
  string temp("{\\bf ");
  temp += s;
  string::size_type pos = temp.find('.');
  if(pos != string::npos) {
    temp.insert(pos+1, "}");
  }
  else {
    temp += "}";
  }
  return temp;
}

string TestReport::boldFaceHtml(const string& s) 
{
  string temp("<strong> ");
  temp += s;
  string::size_type pos = temp.find('.');
  if(pos != string::npos) {
    temp.insert(pos+1, "</strong>");
  }
  else {
    temp += "</strong>";
  }
  return temp;
}

void TestReport::produceNHitPlane2DPlot()
{
  TH2F h2("nHitPlane2D", "number of hit planes distribution", 36, 0.5, 36.5, 16, -0.5, 15.5);
  HistAttribute att("Number of planes that have strip hits", "Tower");
  setHistParameters(&h2, att);

  for(int i = 0; i != g_nTower; ++i) {
    // Take out events with no planes hit:
    for(int j = 1; j != g_nPlane+1; ++j) {

      // For missing towers, all events will have 0 planes that are hit.
      // For clarity of the plot, the corresponding bins won't be filled.
      if(m_nLayer[i]->GetBinContent(j+1) != m_nEvent) {
	h2.Fill(j, i, m_nLayer[i]->GetBinContent(j+1));
      }
    }
  }

  string file(m_prefix);
  file += "_nHitPlane2D";
  PlotAttribute pAtt(file.c_str(), "Number of planes in a tower that have strip hits. Events with no planes hit are not used. If a tower is missing, the information is not included in the plot (bins are not filled).", "nHitPlane2D");
  producePlot(&h2, pAtt);
  insertPlot(pAtt);
}

void TestReport::produceAlignCalTkrPlot()
{
  string file(m_prefix);
  file += "_alignCalTkr";
  PlotAttribute att(file.c_str(), "Distance between the reconstructed CAL cluster XY coordinates and the XY coordinates extrapolated from TKR according to reconstructed track direction. The plot only contains events with more than 50 MeV energy deposited in the CAL.", "alignCalTkr", true);
  att.m_statMode = 111111;
  producePlot(m_alignCalTkr, att);
  insertPlot(att);
}

// ACD digis:
void TestReport::produceAcdDigiPlots()
{

  // divide out some plots to have effics
  Float_t singleVetoRatio = efficDivide(*m_AcdTileIdOneVeto,*m_AcdVetoMap);
  Float_t vetoToHitRatio = efficDivide(*m_AcdVetoMap,*m_AcdHitMap);
  Float_t singlePmtRatio = efficDivide(*m_AcdTileIdOnePMT,*m_AcdHitMap);
  
  string file(m_prefix);

  file += "_nAcdDigis";
  PlotAttribute att(file.c_str(), "Number of ACD digis.  This is just the total number of tiles that have at least one PMT above the zero suppression threshold.", "nAcdDigis");
  att.m_statMode = 111111;
  att.m_yLog = kTRUE;
  producePlot(m_nAcdDigis, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdHitMap";
  att.set(file.c_str(), "ACD Gem ID for all Digis.  Expectect to see spikes at 15,31,47,63.  These are the large tiles at the bottom of the sides of the ACD.", "AcdHitMap" );
  att.m_statMode = 11;
  producePlot(m_AcdHitMap, att);
  insertPlot(att);  

  file = m_prefix;
  file += "_AcdVetoMap";
  att.set(file.c_str(), "Fraction of hits above Veto threshold.  This is the fraction of Digis where at least one of the two PMT have the VETO bit set in the AEM.", "AcdVetoMap" );
  att.m_statMode = 0;
  producePlot(m_AcdVetoMap, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdTileIdOnePMT";
  att.set(file.c_str(), "Fraction of single hits.  This is the fraction of Digis where only one of the two PMT fired.", "AcdTileIdOnePMT");
  att.m_statMode = 0;
  producePlot(m_AcdTileIdOnePMT, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdTileIdOneVeto";
  att.set(file.c_str(), "Fraction of single vetos.  This is the RATIO of digis where only one of the two PMTs was above Veto threshold to the number of digis where either PMT was above the VETO threshold", "AcdTileIdOneVeto");
  att.m_statMode = 0;
  producePlot(m_AcdTileIdOneVeto, att);
  insertPlot(att);
  
  file = m_prefix;
  file += "_AcdPhaMapA";
  att.set(file.c_str(), "ACD PHA map -- A PMTs.  This shows the PHA values for each channel.", "AcdPhaMapA" );
  att.m_zLog = true;
  producePlot(m_AcdPhaMapA, att);
  insertPlot(att);

  file = m_prefix;  
  file += "_AcdPhaMapB";  
  att.set(file.c_str(), "ACD PHA map -- B PMTs.  This shows the PHA values for each channel", "AcdPhaMapB" );
  att.m_zLog = true;
  producePlot(m_AcdPhaMapB, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdGarcGafeHitMap";
  att.set(file.c_str(), "ACD Hit Map by GARC:GAFE.  This shows the digis in electronics space.  Not Attached (NA) channels are marked with *", "AcdGarcGafeHitMap" );
  att.m_zLog = true;
  producePlot(m_AcdGarcGafeHitMap, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdGarcGafeVetoMap";
  att.set(file.c_str(), "ACD Veto Map by GARC:GAFE.  This shows the digis with the AEM veto bit set.  Not Attached (NA) channels are marked with *", "AcdGarcGafeVetoMap" );
  att.m_zLog = true;
  producePlot(m_AcdGarcGafeVetoMap, att);
  insertPlot(att);

}


// ACD recon:
void TestReport::produceAcdTkrReconPlots()
{
  
  // divide out some plots to have effics
  m_AcdEfficMap->Add(m_AcdInEfficMap);

  Float_t inEfficRatio = efficDivide(*m_AcdInEfficMap,*m_AcdEfficMap);

  string file(m_prefix);

  file = m_prefix;
  file += "_AcdInEfficMap";
  
  PlotAttribute att(file.c_str(), "Fraction of tracks extrapolated to tile NOT MATCHED with hits.  By ACD Gem ID.", "AcdInEfficMap");
  att.m_statMode = 0;
  producePlot(m_AcdInEfficMap, att);
  insertPlot(att);  

  file = m_prefix;
  file += "_AcdMissMapTop";
  att.set(file.c_str(), "GLOBAL X-Y postions for track extrapolations NOT MATCHED hits: top of ACD.  You should see the outline of the overlap regions, as this is where the association sometimes fails.  Depending on the sample you may also see a number of events in the central parts fo the tiles.", "AcdMissMapTop" );
  producePlot(m_AcdMissMapTop, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdMissMapMinusX";
  att.set(file.c_str(), "GLOBAL Y-Z postions for track extrapolations NOT MATCHED hits: -X side of ACD.  You should see the outline of the overlap regions, as this is where the association sometimes fails.  Depending on the sample you may also see a number of events in the central parts fo the tiles.", "AcdMissMapMinusX" );
  producePlot(m_AcdMissMapMinusX, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdMissMapMinusY";
  att.set(file.c_str(), "GLOBAL X-Z postions for track extrapolations NOT MATCHED hits: -Y side of ACD.  You should see the outline of the overlap regions, as this is where the association sometimes fails.  Depending on the sample you may also see a number of events in the central parts fo the tiles.", "AcdMissMapMinusY" );
  producePlot(m_AcdMissMapMinusY, att);
  insertPlot(att);
  
  file = m_prefix;
  file += "_AcdMissMapPlusX";
  att.set(file.c_str(), "GLOBAL Y-Z postions for track extrapolations NOT MATCHED hits: +X side of ACD.  You should see the outline of the overlap regions, as this is where the association sometimes fails.  Depending on the sample you may also see a number of events in the central parts fo the tiles.", "AcdMissMapPlusX" );
  producePlot(m_AcdMissMapPlusX, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdMissMapPlusY";
  att.set(file.c_str(), "GLOBAL X-Z postions for track extrapolations NOT MATCHED hits: +Y side of ACD.  You should see the outline of the overlap regions, as this is where the association sometimes fails.  Depending on the sample you may also see a number of events in the central parts fo the tiles.", "AcdMissMapPlusY" );
  producePlot(m_AcdMissMapPlusY, att);
  insertPlot(att);

  file = m_prefix;
  file += "_AcdMipMapA";
  att.set(file.c_str(), "ACD MIPs map -- A PMTs.  This shows the PHA values for each channel calibrated into MIP equivalent (path lenght corrected).", "AcdMipMapA" );
  att.m_zLog = true;
  producePlot(m_AcdMipMapA, att);
  insertPlot(att);

  file = m_prefix;  
  file += "_AcdMipMapB";  
  att.set(file.c_str(), "ACD MIPs map -- B PMTs.  This shows the PHA values for each channel calibrated into MIP equivalent (path lenght corrected).", "AcdMipMapB" );
  att.m_zLog = true;
  producePlot(m_AcdMipMapB, att);
  insertPlot(att);

}


string TestReport::eraseLatexStr(const std::string& s)
{
  string::size_type start = s.find("@latex");
  if(start == string::npos) return s;

  string::size_type end = s.find("@endlatex");
  if(end == string::npos) return s;

  string temp(s);
  temp.erase(start, end-start+9);

  start = temp.find("@html");
  if(start != string::npos) {
    temp.replace(start, 6, "");
  }
  end = temp.find("@endhtml");
  if(end != string::npos) {
    temp.replace(end, 9, "");
  }

  return temp;
}

string TestReport::eraseHtmlStr(const std::string& s)
{
  string::size_type start = s.find("@html");
  if(start == string::npos) return s;

  string::size_type end = s.find("@endhtml");
  if(end == string::npos) return s;

  string temp(s);
  temp.erase(start, end-start+8);

  start = temp.find("@latex");
  if(start != string::npos) {
    temp.replace(start, 6, "");
  }
  end = temp.find("@endlatex");
  if(end != string::npos) {
    temp.replace(end, 9, "");
  }

  return temp;
}
