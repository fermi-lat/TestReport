#include <iostream>
#include <cstdio>
#include "TROOT.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TestReport.h"
#include "ToString.h"

using std::cout;
using std::endl;
using std::string;

TestReport::TestReport(const char* dir, const char* prefix, 
		       const char* version, const char* emVersion)
  : m_dir(dir), m_prefix(prefix), m_version(version), m_emVersion(emVersion),
    m_outputFile(0), m_mcFile(0), m_mcTree(0),
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

  string f(dir);
  f += '/';
  f += prefix;
  f += "_report.doxygen";
  m_report = new ofstream(f.c_str());

  string r(dir);
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

  m_tot = new TH1F("tot", "Tot distribution", 300, 0, 300);
  att.m_xTitle = "Time over threshold";
  att.m_yTitle = "Number of events";
  setHistParameters(m_tot, att);

  for(int i = 0; i != 6; ++i) {
    m_nTrack[i] = 0;
  }
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

  h->SetXTitle(att.m_xTitle.c_str());
  h->SetYTitle(att.m_yTitle.c_str());
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

void TestReport::analyzeReconTree()
{
  TkrRecon* tkrRecon = m_reconEvent->getTkrRecon();
 
  assert(tkrRecon != 0);

  ++m_nTrack[tkrRecon->getTrackCol()->GetLast()+1];
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
    GlastAxis::axis view = tkrDigi->getView();

    ++nDigi[tower];

    ++nLayer[tower];

    nHit[tower] += tkrDigi->getNumHits();

    int lowCount = 0, highCount = 0;
    int nStrips = tkrDigi->getNumHits();
    for(int i = 0; i != nStrips; ++i) {
      int stripId = tkrDigi->getStrip(i);
      if(stripId < 0 || stripId >1535) badStrip = true;

      int div = getGtrcSplit(layer, view);
      (stripId < div) ? ++lowCount : ++highCount;
 
    }

    if(lowCount > 63 || highCount > 63) {
      moreStrip = true;
    }

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

  //  if(tkrTrigger) {
    (maxNDigi >= 6) ? ++m_nEventDigi[6] : ++m_nEventDigi[maxNDigi];
    //  }
}

void TestReport::generateReport()
{
  writeHeader();

  (*m_report) << "@section summary Summary" << endl;
  (*m_report) << "This report is produced using the @b " << m_version 
	      << " TestReport package and the @b " << m_emVersion 
	      << " EngineeringModel package." << endl; 
  (*m_report) << "There are @b " << m_nEvent << " events in @em " 
	      << m_digiFile->GetName() << "." << endl;

  (*m_report) << "@section trigger Trigger" << endl;

  string file(m_prefix);
  file += "_trigger";
  PlotAttribute att(file.c_str(), "Trigger distribution", "trigger", 1);
  producePlot(m_trigger, att);
  insertPlot(att);

  (*m_report) << "@section strip_hit Strip hit info" << endl;

  // produce plots for nhits distribution
  for(int i = 0; i != g_nTower; ++i) {

    if(m_nHit[i]->GetEntries() > 0) {

      char name[] = "_nHit_00";
      sprintf(name, "_nHit_%02d", i);
      file = m_prefix;
      file += name;
      char title[] = "Number of hit strips in tower 00";
      sprintf(title, "Number of hit strips in tower %02d", i);
      char tag[] = "nHit_00";
      sprintf(tag, "nHit_%02d", i);
      PlotAttribute att(file.c_str(), title, tag, 1); 
      producePlot(m_nHit[i], att);
      insertPlot(att);

    }

  }

  // produce plots for nLayer distribution
  for(int i = 0; i != g_nTower; ++i) {

    if(m_nLayer[i]->GetEntries() > 0) {

      char name[] = "_nLayer_00";
      sprintf(name, "_nLayer_%02d", i);
      file = m_prefix;
      file += name;
      char title[] = "Number of hit layers in tower 00";
      sprintf(title, "Number of hit layers in tower %02d", i);
      char tag[] = "nLayer_00";
      sprintf(tag, "nLayer_%02d", i);
      PlotAttribute att(file.c_str(), title, tag, 1); 
      producePlot(m_nLayer[i], att);
      insertPlot(att);

    }

  }

  // produce a table containing number of events as a function a number digis
  TableDef d;

  string caption("Number of events with different number of TKR digis. There are ");
  caption += ToString(m_nTkrTrigger); 
  caption += " events with 3 in a row TKR trigger.";
  d.m_caption = caption.c_str();
  d.m_label = "nEventDigi";

  d.m_nRow = 8;
  d.m_nCol = 3;
  string table[d.m_nRow][d.m_nCol];
  d.m_table = (string*) table;
  table[0][0] = "Number of TKR digis";
  table[0][1] = "Number of TKR events"; 
  table[0][2] = "percentage";

  for(int i = 1; i != d.m_nRow; ++i) {
    if(i == 7) {
      table[i][0] =  ">= 6";
    }
    else {
      table[i][0] = ToString(i-1);
    }
    table[i][1] =  ToString(m_nEventDigi[i-1]);  
    table[i][2] = ToString(float(m_nEventDigi[i-1])/m_nTkrTrigger*100.);
    table[i][2] += "%"; 
  }

  printHtmlTable(d);

  table[7][0] =  "$\\geq$ 6";
  applyDash((string*) table, d.m_nRow*d.m_nCol);
  printLatexTable(d);

  // bad strips
  *(m_report) << "Bad Strips: " << endl;
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  *(m_report) << "@li There are @b " << ToString(m_nEventBadStrip) << " events with strip ID outside the range from 0 to 1535." << endl;

  *(m_report) << "@li There are @b " << ToString(m_nEventMoreStrip) << " events with more than 63 strips per GTRC." << endl << endl;
  
  *(m_report) << "@section tot Time Over Threshold" << endl;

  // bad TOTs
  *(m_report) << "TOT info: " << endl;
  *(m_report) << "@latexonly \\nopagebreak @endlatexonly" << endl;
  *(m_report) << "@li There are @b " << ToString(m_nEventSatTot) << " events with saturated TOT values." << endl;
  *(m_report) << "@li There are @b " << ToString(m_nEventZeroTot) << " events with zero TOT in one plane but nonzero number of strip hits in that plane." << endl;
  *(m_report) << "@li There are @b " << ToString(m_nEventBadTot) << " events with nonzero TOT on one plane but no strip hits in that plane." << endl << endl;

  // TOT plot
  file = m_prefix;
  file += "_tot";
  att.m_file = file.c_str();
  att.m_caption = "TOT distribution";
  att.m_label = "tot"; 
  att.m_yLog = true;
  producePlot(m_tot, att);
  insertPlot(att);

  if(m_reconFile) {

    // produce a table containing number of events as a function of number
    // of reconstructed track
    TableDef dTrack;

    string caption("Number of events as a function of number of reconstructed track. There are ");
    caption += ToString(m_nEvent); 
    caption += " events in the digi root file.";
    dTrack.m_caption = caption.c_str();
    dTrack.m_label = "nTrack";

    dTrack.m_nRow = 7;
    dTrack.m_nCol = 3;
    string table[dTrack.m_nRow][dTrack.m_nCol];
    dTrack.m_table = (string*) table;
    table[0][0] = "Number of reconstructed tracks";
    table[0][1] = "Number of events"; 
    table[0][2] = "percentage";

    for(int i = 1; i != dTrack.m_nRow; ++i) {
      if(i == 6) {
	table[i][0] =  ">= 5";
      }
      else {
	table[i][0] = ToString(i-1);
      }
      table[i][1] =  ToString(m_nTrack[i-1]);  
      table[i][2] = ToString(float(m_nTrack[i-1])/m_nEvent*100.);
      table[i][2] += "%"; 
    }

    printHtmlTable(dTrack);

    table[6][0] =  "$\\geq$ 5";
    applyDash((string*) table, dTrack.m_nRow*dTrack.m_nCol);
    printLatexTable(dTrack);

  }

  writeTail(); 
}

void TestReport::writeHeader()
{
  *(m_report) << "/** @mainpage Run Test Report" << endl
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


void TestReport::producePlot(TH1F* h, const PlotAttribute& att)
{
  TCanvas c1("c1", "produce eps file");
  gPad->SetBorderMode(0);
  gPad->SetFillColor(10);
  if(att.m_yLog) gPad->SetLogy();

  h->Draw();

  string epsFile(m_dir);
  epsFile += "/";
  epsFile += att.m_file;
  epsFile += ".eps";
  c1.SaveAs(epsFile.c_str());

  // ROOT can not produce gif file in the batch mode
  // run pstopnm and ppmtogif to produce gif file

  string pwd = gSystem->WorkingDirectory();
  gSystem->cd(m_dir.c_str());

  string cmd1("pstopnm -ppm -xborder 0 -yborder 0 -portrait ");
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

  *(m_report) << "@image html " << gifFile << " \"" << att.m_caption << "\" "
	      << endl;
 
  string epsFile(att.m_file);
  epsFile += ".eps";

  *(m_report) << "@image latex " << epsFile << " \"" << att.m_caption << "\" "
	      << "height=" << att.m_height << "cm" << ",width=" 
	      << att.m_width << "cm " << endl << endl;
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
  *(m_report) << "\\begin{table}" << endl;
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

int TestReport::getGtrcSplit(int layer, GlastAxis::axis view)
{
  // for EM2
  if(layer == 1 && view == GlastAxis::X) {
    return g_nStrip / g_nFEC * 4;
  }
  else {
    return g_nStrip / 2;
  }
}
