#ifndef TestReport_Header
#define TestReport_Header

#include <fstream>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TH2F.h"
#include "mcRootData/McEvent.h"
#include "reconRootData/ReconEvent.h"
#include "digiRootData/DigiEvent.h"

/**
 * \class TestReport
 * \brief TestReport produces SVAC reports for both digi and recon data
 * \author Xin Chen
 */

class TestReport {

 public:

  TestReport(const char* dir, const char* prefix, const char* version,
	     const char* emVersion, const char*tkrCalibSerNo, 
	     const char* calCalibSerNo);
  ~TestReport();

  void analyzeTrees(const char* mcFileName,
		    const char* digiFileName,
		    const char* reconFileName);

  void generateReport();

 private:

  void generateDigiReport();

  void generateReconReport();

  struct Attribute {

    Attribute(const char* xTitle="", const char* yTitle="", 
	      float axisTitleSize=0.04, float axisLabelSize=0.04, 
	      float xTitleOffset=1, float yTitleOffset=1) : 
      m_xTitle(xTitle), m_yTitle(yTitle), m_axisTitleSize(axisTitleSize), 
	 m_axisLabelSize(axisLabelSize), m_xTitleOffset(xTitleOffset),
	 m_yTitleOffset(yTitleOffset) { }

    virtual ~Attribute() {}

    virtual void set(const char* xTitle="", const char* yTitle="", 
		     float axisTitleSize=0.04, float axisLabelSize=0.04, 
		     float xTitleOffset=1, float yTitleOffset=1)  
    {
      m_xTitle = xTitle;
      m_yTitle = yTitle;
      m_axisLabelSize = axisLabelSize;
      m_axisTitleSize = axisTitleSize;
      m_xTitleOffset = xTitleOffset;
      m_yTitleOffset = yTitleOffset;
    }
    std::string m_xTitle;
    std::string m_yTitle;
    float m_axisTitleSize;
    float m_axisLabelSize;
    float m_xTitleOffset;
    float m_yTitleOffset;
  };

  struct HistAttribute : public Attribute{

    HistAttribute(const char* xTitle="", const char* yTitle="", 
		  float axisTitleSize=0.04, float axisLabelSize=0.04, 
		  float xTitleOffset=1, float yTitleOffset=1,
		  bool canRebin=true) : 
      Attribute(xTitle, yTitle, axisTitleSize, axisLabelSize, xTitleOffset,
		yTitleOffset), m_canRebin(canRebin) { }

    void set(const char* xTitle="", const char* yTitle="", 
	     float axisTitleSize=0.04, float axisLabelSize=0.04, 
	     float xTitleOffset=1, float yTitleOffset=1, bool canRebin=true)  
    {
      Attribute::set(xTitle, yTitle, axisTitleSize, axisLabelSize, 
		     xTitleOffset, yTitleOffset);
      m_canRebin = canRebin;
    }

    bool m_canRebin;
  };

  struct GraphAttribute : public Attribute{

    GraphAttribute(const char* xTitle="", const char* yTitle="", 
		   float axisTitleSize=0.04, float axisLabelSize=0.04, 
		   float xTitleOffset=1, float yTitleOffset=1,
		   float markerSize=1) : 
      Attribute(xTitle, yTitle, axisTitleSize, axisLabelSize, xTitleOffset,
		yTitleOffset), m_markerSize(markerSize) { }

    void set(const char* xTitle="", const char* yTitle="", 
	     float axisTitleSize=0.04, float axisLabelSize=0.04, 
	     float xTitleOffset=1, float yTitleOffset=1, 
	     float markerSize=1)  
    {
      Attribute::set(xTitle, yTitle, axisTitleSize, axisLabelSize, 
		     xTitleOffset, yTitleOffset);
      m_markerSize = markerSize;
    }

    float m_markerSize;
  };

  struct PlotAttribute {
    PlotAttribute::PlotAttribute(const char* file=0, const char* caption=0,
				 const char* label=0, bool yLog=0,
				 float height=10, float width=15, int x=606,
				 int y=410, int stat=1111) : 
      m_file(file), m_caption(caption), m_label(label), m_yLog(yLog),
	 m_height(height), m_width(width), m_xPixel(x), m_yPixel(y),
	 m_statMode(stat)
    { 
      m_2dPlotType = COLZ;
      m_nColor = 3;
      m_colors[0] = 1; 
      m_colors[1] = 5;
      m_colors[2] = 2;
      m_colors[3] = 15;
      m_colors[4] = 30;
      m_colors[5] = 4;
    }

    void set(const char* file=0, const char* caption=0, const char* label=0, 
	     bool yLog=0, float height=10, float width=15, int x=606, 
	     int y=410, int stat=1111)
    {
      m_file = file;
      m_caption = caption;
      m_label = label;
      m_yLog = yLog;
      m_height = height;
      m_width = width;
      m_xPixel = x;
      m_yPixel = y;
      m_nColor = 3;
      m_2dPlotType = COLZ;
      m_statMode = stat;
    }

    /// name of file to be produced, note file type such as ".eps" is not
    /// included 
    const char* m_file;  
    const char* m_caption;
    const char* m_label;
    bool m_yLog;

    /// height and width of a plot in latex
    float m_height;
    float m_width;

    /// height and width in pixel for a gif plot on web
    int m_xPixel, m_yPixel;

    enum PlotType {COLZ, LEGO};
    PlotType m_2dPlotType;

    /// colors used to make a palette for drawing 2D histogram using colz
    int m_nColor;
    int m_colors[6];

    /// mode controlling what are drawn in the stat box
    int m_statMode;
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

  /// print table in html format
  void printHtmlTable(const TableDef& r);

  /// print table in latex format
  void printLatexTable(const TableDef& r);

  void printGltTriggerTable();

  void printCondSummaryTable();

  /// produce a table containing number of events as a function of number of digis
  void printNDigiTable();

  /// produce a table containing ratio of events with TKR trigger but less than 6 digis in each tower
  void printNDigiTowerTable();

  void printNTrackTable();

  /// produce a plot describing ratio of events with TKR trigger but less than 6 digis
  void produceNDigiPlot();

  /// produce average TOT plots in each plane, each tower
  void produceAveTotPlots();
 
  /// produce 2D average TOT plots in each plane, each tower
  void produceAveTot2DPlot();

  /// produce 2D nhits plot in each plane, each tower
  void produceNhits2DPlot();

  /// produce 2D nhits plot in each crystal
  void produceCalNhits2DPlot();

  /// produce 2D plot for number of events with zero or saturated tot value in each plane of each tower
  void produceZeroSatTot2DPlot();

  void produceTotNoise2DPlot();

  /// produce a histogram plot of sum of CAL energies
  void produceCalEneSum2DPlot();

  /// produce 2D plot of energies measured in each CAL layer
  void produceCalEneLayer2DPlot();

  void produceReconDirPlots();

  void produceReconPosPlots();

  void produceReconEnePlots();

  void produceTimeIntervalPlot();

  void produceNHitPlane2DPlot();

  /// produce plots of histogram m_alignCalTkr
  void produceAlignCalTkrPlot();

  /// set some common parameters for a 1D histogram
  void setHistParameters(TH1* h, const HistAttribute& att);

  /// set some common parameters for a graph
  void setGraphParameters(TGraph* h, const GraphAttribute& att);

  /// write Latex header
  void writeHeader();

  /// write Latex tail
  void writeTail();

  /// produce a eps file from a 1D, 2D histogram or a graph
  void producePlot(TObject* h, const PlotAttribute& att);

  /// produce a eps file from several 1D, 2D histograms or graphs
  void producePlot(TObject** h, const PlotAttribute& att, int nRow, int nCol);

  void insertPlot(const PlotAttribute& att);

  /// scale a 2d histogram by number of events in each bin
  /// note size and dimensions of nEvents array must agree with the histogram
  void scale2DHist(TH2F* h, int* nEvents);

  /// apply \ in front of some latex special characters 
  void applyDash(std::string* x, int n) const;

  /// used in producing caption for a plot in a latex file. It will put the
  /// first sentence in the caption to be bold face.
  std::string boldFaceLatex(const std::string& s);

  /// used in producing caption for a plot in a html file. It will put the
  /// first sentence in the caption to be bold face.
  std::string boldFaceHtml(const std::string& s);

  /// delete sub strings enclosed between @latext and @latex
  std::string eraseLatexStr(const std::string& s);

  /// delete sub strings enclosed between @html and @html
  std::string eraseHtmlStr(const std::string& s);

  void analyzeMcTree() { }

  void analyzeReconTree();

  void analyzeDigiTree();

  std::ofstream* m_report;

  /// directory where all report files are generated
  std::string m_dir;

  /// prefix to the report files
  std::string m_prefix;

  /// version of the TestReport package
  std::string m_version;

  /// version of the EngineeringModel package
  std::string m_emVersion;
 
  std::string m_tkrCalibSerNo;
  std::string m_calCalibSerNo;

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
	g_nFEC = 24, g_nTower = 16, g_satTot = 255, g_nCalLayer = 8,
	g_nEnd =2};

  /// trigger histogram
  TH1F* m_trigger;

  /// no of bad events indicated in event summary data
  unsigned m_nBadEvts;

  /// condition summary in GEM
  TH1F* m_condSummary;

  /// number of events in the digi root file
  int m_nEvent;

  /// number of events with 3 in a row trigger in GEM
  int m_nTkrTrigger;

  /// number of events with different number of digis.
  /// For example: m_nEventDigi[0] is number of events with 0 digi
  /// m_nEventDigi[6] is number of events with >= 6 digis
  /// If there are more than one tower, maximal number of digis in any single
  /// tower is used
  int m_nEventDigi[7];

  /// number of events with strip ID outside the range from 0 to 1535
  int m_nEventBadStrip;

  /// number of events with more than 63 strips per GTRC
  int m_nEventMoreStrip;

  /// number of strip hits for each tower
  TH1F* m_nHit[g_nTower];

  /// average number of strip hits in a particular TKR plane. 
  /// zero hit is not used in calculating the average 
  TH2F* m_nHit2D;

  /// accumulated number of strip hits in a particular TKR plane. 
  /// zero hit is not used in calculating the average, used to fill m_nHit2D
  int m_nHitPlane[g_nTower][g_nPlane];

  /// no of events with zero strip hit in a particular TKR plane
  TH2F* m_nZeroHit2D;

  /// no of events with non-zero strip hit in one end of a particular TKR plane
  /// end = 0 corresponds to GTRC value close to strip 0
  int m_nEvtHitPlane[g_nTower][g_nPlane][g_nEnd];

  /// no. of events with non-zero strip hit in one plane
  int m_nEvtPlane[g_nTower][g_nPlane];

  /// average number of crystal hits in a particular CAL layer 
  /// 0 hit is not used in calculating the average
  TH2F* m_nCalHit2D;

  /// average number of crystal hits in a particular CAL layer 
  /// used to fill m_nCalHit2D
  int m_nCalHit[g_nTower][g_nCalLayer];

  /// ratio of events with no hit in a particular CAL layer
  TH2F* m_nZeroCalHit2D;

  /// no of events with nonzero hit in a particular CAL layer
  int m_nEvtCalHit[g_nTower][g_nCalLayer];

  /// number of single hit layers (range [0, 36], 0 is at the bottom) for each 
  /// tower
  TH1F* m_nLayer[g_nTower];

  /// distribution of events with zero TOT
  TH2F* m_totZero2D;

  /// distribution of events with saturated TOT
  TH2F* m_totSat2D;

  /// distribution of average TOT values excluding 0 and saturation
  TH2F* m_totAve2D;

  /// 2d histogram of ratio of events with zero TOT but no-zero strip hits in
  /// a particular plane. Likely to be noises.
  TH2F* m_totNoise2D;

  /// TOT distributions
  TH1F* m_tot[g_nTower][g_nPlane][2];

  /// number of events with saturated TOT (250 ADC )
  int m_nEventSatTot;

  /// number of events with 0 TOT but at least 1 strip
  int m_nEventZeroTot;

  /// number of events with TOT values outside range [0, g_satTot]
  int m_nEvtInvalidTot;

  /// number of events with none zero TOT but no strip hit
  int m_nEventBadTot;

  /// time of first trigger
  UInt_t m_startTime;

  /// time of last trigger
  UInt_t m_endTime;

  /// histogram of time between adjacent event in mili second
  TH1F* m_timeInterval;

  /// histogram of time between adjacent event in mili second with a cut of
  /// 3 ms to study the dead time
  TH1F* m_timeIntervalCut;

  /// histogram of time between adjacent event in mili second, taken from GEM
  TH1F* m_timeIntervalGem;

  /// percentage of events with TKR trigger but less than 6 digis in a tower
  TGraph* m_nDigi;

  /// no. of events in each tower with TKR trigger
  int m_nTkrEvent[g_nTower];

  /// no. of events in each tower with TKR trigger and less than 6 digis
  int m_nTkrBadEvent[g_nTower];

  TH1F* m_nTkrTrack;

  /// angular distributions of fit tracks
  TH2F* m_reconDirXY;
  TH1F* m_reconDirZ;

  /// position distribution of fit vertices
  TH2F* m_reconPosXY;
  TH1F* m_reconPosZ;

  /// reconstructed energy stored in first reconstructed vertex
  TH1F* m_reconEne;

  /// sum of energy in the CAL cluster, at the moment there is only one cluster
  TH1F* m_calSumEne;

  /// average energy measured in each crystal layer, zero energy is not used 
  /// in calculating average
  TH2F* m_calEneLayer2D; 

  /// average energy measured in each crystal layer, zero energy is not used 
  /// in calculating average. Used to fill m_calEneLayer2D
  double m_calEneLayer[g_nTower][g_nCalLayer];

  /// no of events where energy measured in a particular layer is zero
  TH2F* m_zeroCalEneLayer2D; 

  /// no of events where non-zero energy is measured in a particular layer.
  int m_nCalEneLayer[g_nTower][g_nCalLayer];

  /// TKR split info. The array contains no. of cards read from each end
  int m_nFec[g_nTower][g_nLayer][g_nView][2];

  /// histograms containing distance between reconstructed CAL cluster xy 
  /// position and the position extrapolated from the reconstructed track 
  /// in TKR
  TH1F* m_alignCalTkr;
};
#endif
