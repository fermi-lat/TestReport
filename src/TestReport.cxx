#include <iostream>
#include <cstdio>
#include "TROOT.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TestReport.h"
#include "ToString.h"

TestReport::TestReport(const char* dir, const char* prefix)
  : m_dir(dir), m_prefix(prefix), m_outputFile(0), m_mcFile(0), m_mcTree(0),
    m_mcBranch(0), m_mcEvent(0), m_reconFile(0), m_reconTree(0), 
    m_reconBranch(0), m_reconEvent(0), m_digiFile(0), m_digiTree(0),
    m_digiBranch(0), m_digiEvent(0), m_trigger(0), m_nEvent(0),
    m_nTkrTrigger(0), m_nEventBadStrip(0), m_nEventMoreStrip(0), 
    m_nEventSatTot(0), m_nEventZeroTot(0), m_nEventBadTot(0)
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

  std::string f(dir);
  f += '/';
  f += prefix;
  f += "_report.txt";
  m_report = new ofstream(f.c_str());

  std::string r(dir);
  r += '/';
  r += prefix;
  r += "_hist.root";
  m_outputFile = new TFile(r.c_str(), "RECREATE");

  m_trigger = new TH1F("trigger", "Trigger distribution", 32, 0, 32);
  HistAttribute att("Trigger word", "Number of events");
  setHistParameters(m_trigger, att);

  for(int i = 0; i != 7; ++i) {
    m_nEventDigi[i] = 0;
  }

  for(int i = 0; i != g_nTower; ++i) {

    char name[] = "nHitss00";
    sprintf(name, "nHitss%02d", i);
    char title[] = "Number of strip hits in tower 00";
    sprintf(title, "Number of strip hits in tower %02d", i);
    m_nHit[i] = new TH1F(name, title, 19, 1, 20);
    att.m_xTitle = "Number of strip hits";
    att.m_yTitle = "Number of events";
    setHistParameters(m_nHit[i], att);
 
    char name1[] = "nLayer00";
    sprintf(name1, "nLayer%02d", i);
    char title1[] = "Number of hit layers in tower 00";
    sprintf(title1, "Number of hit layers in tower %02d", i);
    m_nLayer[i] = new TH1F(name1, title1, 17, 1, 18);
    att.m_xTitle = "Number of hit layers";
    att.m_yTitle = "Number of events";
    setHistParameters(m_nLayer[i], att);

  }

  m_tot = new TH1F("tot", "Tot distribution", 251, 0, 250);
  att.m_xTitle = "Time over threshold";
  att.m_yTitle = "Number of events";
  setHistParameters(m_trigger, att);

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

void TestReport::setHistParameters(TH1F* h, const HistAttribute& att)
{
  assert(h != 0);

  h->SetXTitle(att.m_xTitle);
  h->SetYTitle(att.m_yTitle);
  h->SetTitle("");
  h->SetLineWidth(1);
  h->SetBit(TH1::kCanRebin);
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

  int nMc, nRecon, nDigi;
  if(m_mcFile) {
    nMc = (int) m_mcTree->GetEntries();
    std::cout << "No of events in " << mcFileName << " : " << nMc << std::endl;
    m_nEvent = nMc;
  }
  if(m_reconFile) {
    nRecon = (int) m_reconTree->GetEntries();
    std::cout << "No of events in " << reconFileName << " : " << nRecon << std::endl;
    m_nEvent = nRecon;
  }
  if(m_digiFile) {
    nDigi = (int) m_digiTree->GetEntries();
    std::cout << "No of events in " << digiFileName << " : " << nDigi << std::endl;
    m_nEvent = nDigi;
  }

  for(int iEvent = 0; iEvent != m_nEvent; ++iEvent) {

    if(m_mcFile) {
      m_mcBranch->GetEntry(iEvent);
      analyzeMcTree();
    }

    if(m_digiFile) {
      m_digiBranch->GetEntry(iEvent);
      analyzeDigiTree();
    }

    if(m_reconFile) {
      m_reconBranch->GetEntry(iEvent);
      analyzeReconTree();
    }

  }  
  
  if(m_mcFile) m_mcFile->Close();
  if(m_reconFile) m_reconFile->Close();
  if(m_digiFile) m_digiFile->Close();
}

void TestReport::analyzeDigiTree()
{
  int trigger = m_digiEvent->getL1T().getTriggerWord();
  m_trigger->Fill(trigger);

  // 3 in a row TKR trigger
  bool tkrTrigger;
  if(trigger & 4) {
    ++m_nTkrTrigger;
    tkrTrigger = true;
  }
  else {
    tkrTrigger = false;
  }

  int nTkrDigis = m_digiEvent->getTkrDigiCol()->GetLast()+1;

  int nHit[g_nTower], nLayer[g_nTower], nDigi[g_nTower];
  for(int i = 0; i != g_nTower; ++i) {
    nHit[i] = 0;
    nLayer[i] = 0;
    nDigi[i] = 0;
  }

  bool badStrip = false, moreStrip = false;
  bool satTot = false, zeroTot = false, badTot = false;

  for(int i = 0; i != nTkrDigis; ++i) {

    const TkrDigi* tkrDigi = m_digiEvent->getTkrDigi(i);
    assert(tkrDigi != 0); 

    int tower = tkrDigi->getTower().id();
    int layer = tkrDigi->getBilayer();

    ++nDigi[tower];

    ++nLayer[tower];

    nHit[tower] += tkrDigi->getNumHits();

    int lowCount = 0, highCount = 0;
    int nStrips = tkrDigi->getNumHits();
    for(int i = 0; i != nStrips; ++i) {
      int stripId = tkrDigi->getStrip(i);
      if(stripId < 0 || stripId >1535) badStrip = true;

      int div = getGtrcSplit(layer);
      (stripId < div) ? ++lowCount : ++highCount;
 
    }

    if(lowCount > 63 || highCount > 63) moreStrip = true;

    int tot0 = tkrDigi->getToT(0);
    int tot1 = tkrDigi->getToT(1);

    if((tot0> 0 || tot1> 0) && nStrips == 0) badTot = true;
    if(tot0 <= 0 && tot1 <= 0 && nStrips > 0) zeroTot = true;
    
    assert(tot0>=0 && tot0 <=g_satTot && tot1>=0 && tot1 <=g_satTot); 
    if(tot0 == g_satTot || tot1 == g_satTot) satTot = true;

    m_tot->Fill(tot0);
    m_tot->Fill(tot1);

  }

  if(badStrip) ++m_nEventBadStrip;
  if(moreStrip) ++m_nEventMoreStrip;
  if(badTot) ++m_nEventBadTot;
  if(zeroTot) ++m_nEventZeroTot;
  if(satTot) ++m_nEventSatTot;

  int maxNDigi = 0;
  for(int i = 0; i != g_nTower; ++i) {
    if(nHit[i] > 0) m_nHit[i]->Fill(nHit[i]);
    if(nLayer[i] > 0) m_nLayer[i]->Fill(nLayer[i]);
    if(nDigi[i] > maxNDigi) maxNDigi = nDigi[i];
  }

  if(tkrTrigger) {
    (maxNDigi >= 6) ? ++m_nEventDigi[6] : ++m_nEventDigi[maxNDigi];
  }
}

void TestReport::generateReport()
{
  writeHeader();

  (*m_report) << "\\section{Summary}" << std::endl;
  (*m_report) << "There are {\\bf " << m_nEvent << "} events in " 
	      << applyDash(m_digiFile->GetName()) << "."
	      << std::endl;

  (*m_report) << "\\section{Trigger}" << std::endl;

  std::string epsFile(m_dir);
  epsFile += '/';
  epsFile += m_prefix;
  epsFile += "_trigger.eps";
  PlotAttribute att(epsFile.c_str(), "Trigger distribution", "trigger", 1);
  producePlot(m_trigger, att);
  insertPlot(att);

  (*m_report) << "\\section{Strip hit info}" << std::endl;

  // produce plots for nhits distribution
  for(int i = 0; i != g_nTower; ++i) {

    if(m_nHit[i]->GetEntries() > 0) {

      char name[] = "_nHit_00";
      sprintf(name, "_nHit_%02d", i);
      epsFile = m_dir;
      epsFile += '/';
      epsFile += m_prefix;
      epsFile += name;
      epsFile += ".eps";
      char title[] = "Number of hit strips in tower 00";
      sprintf(title, "Number of hit strips in tower %02d", i);
      char tag[] = "nHit_00";
      sprintf(tag, "nHit_%02d", i);
      PlotAttribute att(epsFile.c_str(), title, tag, 1); 
      producePlot(m_nHit[i], att);
      insertPlot(att);

    }

  }

  // produce plots for nLayer distribution
  for(int i = 0; i != g_nTower; ++i) {

    if(m_nLayer[i]->GetEntries() > 0) {

      char name[] = "_nLayer_00";
      sprintf(name, "_nLayer_%02d", i);
      epsFile = m_dir;
      epsFile += '/';
      epsFile += m_prefix;
      epsFile += name;
      epsFile += ".eps";
      char title[] = "Number of hit layers in tower 00";
      sprintf(title, "Number of hit layers in tower %02d", i);
      char tag[] = "nLayer_00";
      sprintf(tag, "nLayer_%02d", i);
      PlotAttribute att(epsFile.c_str(), title, tag, 1); 
      producePlot(m_nLayer[i], att);
      insertPlot(att);

    }

  }

  // produce a table containing number of events as a function a number digis
  *(m_report) << "\\begin{table}" << std::endl
	      << "\\begin{center}" << std::endl
	      << "\\caption{Number of events with different numer of TKR digis. There are " << ToString(m_nTkrTrigger) << " events with 3 in a row TKR trigger.}" << std::endl
	      << "\\label{nEventDigi}" << std::endl
	      << "\\begin{tabular}{|c|c|c|} \\hline" << std::endl
	      << "Number of TKR digis & Number of TKR events & percentage \\\\ \\hline" << std::endl;

  for(int i = 0; i != 7; ++i) {
    if(i == 6) {
      *(m_report) << "$\\geq$ 6 & " << m_nEventDigi[i] << " & " 
		  << float(m_nEventDigi[i])/m_nTkrTrigger*100. << "\\%" 
		  << "\\\\ \\hline" << std::endl;
    }
    else {
      *(m_report) << i << " & " << m_nEventDigi[i] << " & " 
		  << float(m_nEventDigi[i])/m_nTkrTrigger*100. << "\\%" 
		  << "\\\\ \\hline" << std::endl;
    }
  }
  *(m_report) << "\\end{tabular}" << std::endl
              << "\\end{center}" << std::endl
	      << "\\end{table}" << std::endl << std::endl;

  // bad strips
  *(m_report) << "Bad Strips: \\nopagebreak" << std::endl;
  *(m_report) << "\\begin{itemize}" << std::endl;
  *(m_report) << "\\item There are {\\bf " << ToString(m_nEventBadStrip) << "} events with strip ID outside the range from 0 to 1535." << std::endl;

  *(m_report) << "\\item There are {\\bf " << ToString(m_nEventMoreStrip) << "} events with more than 63 strips per GTRC." << std::endl;
  *(m_report) << "\\end{itemize}" << std::endl;

  *(m_report) << "\\section{Time Over Threshold}" << std::endl;

  // bad TOTs
  *(m_report) << "TOT info: \\nopagebreak" << std::endl;
  *(m_report) << "\\begin{itemize}" << std::endl;
  *(m_report) << "\\item There are {\\bf " << ToString(m_nEventSatTot) << "} events with saturated TOT values." << std::endl;
  *(m_report) << "\\item There are {\\bf " << ToString(m_nEventZeroTot) << "} events with zero TOT on one plane but nonzero number of strip hits in that plane." << std::endl;
  *(m_report) << "\\item There are {\\bf " << ToString(m_nEventBadTot) << "} events with nonzero TOT on one plane but no strip hits in that plane." << std::endl;
  *(m_report) << "\\end{itemize}" << std::endl;

  // TOT plot
  epsFile = m_dir;
  epsFile += '/';
  epsFile += m_prefix;
  epsFile += "_tot.eps";
  att.m_epsFile = epsFile.c_str();
  att.m_caption = "TOT distribution";
  att.m_label = "tot"; 
  att.m_yLog = true;
  producePlot(m_tot, att);
  insertPlot(att);

  writeTail(); 
}

void TestReport::writeHeader()
{
  *(m_report) << "\\documentstyle[12pt,epsfig]{article}" << std::endl
	      << "\\textwidth 150mm" << std::endl
	      << "\\textheight 242mm" << std::endl
	      << "\\hoffset -15mm" << std::endl
	      << "\\voffset -20mm" << std::endl << std::endl
	      << "\\begin{document}" << std::endl
	      << "\\title{Run Test Report}" << std::endl
	      << "\\author{automatically generated}" << std::endl
	      << "\\maketitle" << std::endl << std::endl 
	      << "\\newpage" << std::endl;
}

void TestReport::writeTail()
{
  *(m_report) << "\\end{document}" << std::endl;
}


void TestReport::producePlot(TH1F* h, const PlotAttribute& att)
{
  TCanvas c1("c1", "produce eps file");
  gPad->SetBorderMode(0);
  gPad->SetFillColor(10);
  if(att.m_yLog) gPad->SetLogy();

  h->Draw();

  c1.Print(att.m_epsFile);
}

void TestReport::insertPlot(const PlotAttribute& att)
{
  *(m_report) << "\\begin{figure}" << std::endl
	      << "\\centerline{\\epsfig{figure=" << att.m_epsFile
	      << ", height=" << att.m_height << "cm, width=" << att.m_width
	      << "cm}}" << std::endl
	      << "\\caption{" << att.m_caption << "}" << std::endl
	      << "\\label{" << att.m_label << "}" << std::endl
	      << "\\end{figure}" << std::endl << std::endl; 
}

std::string TestReport::applyDash(const char* x) const 
{
  std::string s;

  for(const char* p = x; *p != 0; ++p) {
    if(*p == '_') s += "\\";
    s += *p;
  }

  return s;
}      
