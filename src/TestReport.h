#ifndef TestReport_Header
#define TestReport_Header

#include <fstream>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "mcRootData/McEvent.h"
#include "reconRootData/ReconEvent.h"
#include "digiRootData/DigiEvent.h"

class TestReport {

 public:

  TestReport(const char* dir, const char* prefix);
  ~TestReport();

  void analyzeTrees(const char* mcFileName,
		    const char* digiFileName,
		    const char* reconFileName);

  void generateReport();

 private:

  struct HistAttribute {
    HistAttribute(const char* xTitle=0, const char* yTitle=0) : 
    m_xTitle(xTitle), m_yTitle(yTitle) { }
    const char* m_xTitle;
    const char* m_yTitle;
  };

  struct PlotAttribute {
    PlotAttribute::PlotAttribute(const char* file=0, const char* caption=0,
				 const char* label=0, bool yLog=0,
				 float height=10, float width=15) : 
      m_file(file), m_caption(caption), m_label(label), m_yLog(yLog),
	 m_height(height), m_width(width)
    { }
    // name of file to be produced, note file type such as ".eps" is not
    // included 
    const char* m_file;  
    const char* m_caption;
    const char* m_label;
    bool m_yLog;
    float m_height;
    float m_width;
  };

  struct TableDef {
    TableDef::TableDef() { }
    TableDef::TableDef(const std::string* table, const char* caption, 
		       const char* label, int nRow, int nCol) :
      m_table(table), m_caption(caption), m_label(label), m_nRow(nRow), 
	 m_nCol(nCol) { }

    const std::string* m_table;
    const char* m_caption;
    const char* m_label;
    int m_nRow;
    int m_nCol;
  };

  // print table in html format
  void printHtmlTable(const TableDef& r);

  //print table in latex format
  void printLatexTable(const TableDef& r);

  //set some common parameters for a 1D histogram
  void setHistParameters(TH1F* h, const HistAttribute& att);

  // write Latex header
  void writeHeader();

  // write Latex tail
  void writeTail();

  // produce a eps file from a 1D histogram
  void producePlot(TH1F* h, const PlotAttribute& att);

  void insertPlot(const PlotAttribute& att);

  // apply \ in front of some latex special characters 
  void applyDash(std::string* x, int n) const;

  int getGtrcSplit(int layer) { return 1536/2; }

  void analyzeMcTree() { }

  void analyzeReconTree() { }

  void analyzeDigiTree();

  std::ofstream* m_report;

  // directory where all report files are generated
  std::string m_dir;

  // prefix to the report files
  std::string m_prefix;

  TFile* m_outputFile;

  TFile* m_mcFile;
  TTree* m_mcTree;
  TBranch* m_mcBranch;
  McEvent* m_mcEvent;

  TFile* m_reconFile;
  TTree* m_reconTree;
  TBranch* m_reconBranch;
  ReconEvent* m_reconEvent;

  TFile* m_digiFile;
  TTree* m_digiTree;
  TBranch* m_digiBranch;
  DigiEvent* m_digiEvent;

  enum {g_nLayer = 18, g_nView = 2, g_nPlane = 36, g_nStrip = 1536, 
	g_nFEC = 24, g_nTower = 16, g_satTot = 250};

  // trigger histogram
  TH1F* m_trigger;

  // number of events in the digi root file
  int m_nEvent;

  // number of events with 3 in a row trigger
  int m_nTkrTrigger;

  // number of events with different number of digis.
  // For example: m_nEventDigi[0] is number of events with 0 digi
  // m_nEventDigi[6] is number of events with >= 6 digis
  // If there are more than one tower, maximal number of digis in any single
  // tower is used
  int m_nEventDigi[7];

  // number of events with strip ID outside the range from 0 to 1535
  int m_nEventBadStrip;

  // number of events with more than 63 strips per GTRC
  int m_nEventMoreStrip;

  // number of strip hits for each tower
  TH1F* m_nHit[g_nTower];

  // number of layers for each tower
  TH1F* m_nLayer[g_nTower];

  // tot distribution
  TH1F* m_tot;

  // number of events with saturated TOT (250 ADC )
  int m_nEventSatTot;

  // number of events with 0 TOT but at least 1 strip
  int m_nEventZeroTot;

  // number of events with none zero TOT but no strip hit
  int m_nEventBadTot;

};
#endif
