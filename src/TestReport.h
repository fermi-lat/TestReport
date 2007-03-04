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
#include "enums/TriggerBits.h"
#include "reconRootData/AcdRecon.h"
#include "reconRootData/AcdTkrIntersection.h"
#include "digiRootData/AcdDigi.h"

/**
 * \class TestReport
 * \brief TestReport produces SVAC reports for both digi and recon data
 * \author Xin Chen
 */

class TestReport {

 public:

  static Float_t efficDivide(TH1& top, const TH1& bottom, Bool_t inEffic = kFALSE);

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
		  bool canRebin=true,  bool use2DStat=false) : 
      Attribute(xTitle, yTitle, axisTitleSize, axisLabelSize, xTitleOffset,
		yTitleOffset), m_canRebin(canRebin), m_use2DStat(use2DStat) { }

    void set(const char* xTitle="", const char* yTitle="", 
	     float axisTitleSize=0.04, float axisLabelSize=0.04, 
	     float xTitleOffset=1, float yTitleOffset=1, bool canRebin=true, bool use2DStat=false)  
    {
      Attribute::set(xTitle, yTitle, axisTitleSize, axisLabelSize, 
		     xTitleOffset, yTitleOffset);
      m_canRebin = canRebin;
      m_use2DStat = use2DStat;
    }

    bool m_canRebin;
    bool m_use2DStat;
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
				 const char* label=0, bool yLog=0, bool zLog=0,
				 float height=10, float width=15, int x=606,
				 int y=410, int stat=1111, bool statBox=0) : 
      m_file(file), m_caption(caption), m_label(label), m_yLog(yLog), m_zLog(zLog),
	 m_height(height), m_width(width), m_xPixel(x), m_yPixel(y),
	 m_statMode(stat), m_statBox(statBox)
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
	     bool yLog=0, bool zLog=0, float height=10, float width=15, int x=606, 
	     int y=410, int stat=1111, bool statBox=0)
    {
      m_file = file;
      m_caption = caption;
      m_label = label;
      m_yLog = yLog;
      m_zLog = zLog;
      m_height = height;
      m_width = width;
      m_xPixel = x;
      m_yPixel = y;
      m_nColor = 3;
      m_2dPlotType = COLZ;
      m_statMode = stat;
      m_statBox = statBox;
    }

    /// name of file to be produced, note file type such as ".eps" is not
    /// included 
    const char* m_file;  
    const char* m_caption;
    const char* m_label;
    bool m_yLog;
    bool m_zLog;

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
    bool m_statBox;
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

  void produceEpuPlot();

  void produceGemDiscardedPlot();

  void produceTriggerPerTowerPlot();

  void produceCondArrivalTimesPlots();

  void produceAcdTriggerPlots();

  void produceTimeIntervalPlotSBC();
  void produceTimeIntervalPlotGEM();

  void produceTriggerRatePlot(); 

  void produceNHitPlane2DPlot();

  /// produce plots of histogram m_alignCalTkr
  void produceAlignCalTkrPlot();

  /// Produce plots of histogram m_nAcdDigis
  void produceAcdDigiPlots();

  /// Produce plots of histogram m_nAcdDigis
  void produceAcdTkrReconPlots();

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

  /// convert to the dense acd notation
  static UShort_t getGemId(UInt_t id);
    
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
	g_nFEC = 24, g_nTower = 16, g_satTot = 250, g_overlapTot = 255,
        g_nCalLayer = 8, g_nEnd =2};

  /// trigger histogram
  TH1F* m_trigger;

  /// no of bad events indicated in event summary data
  unsigned int m_nBadEvts;

  // LATTE?
  int m_isLATTE;

  // Bay 10, layer 0 split:
  int m_bay10Layer0SplitDefault;

  // Ground ID:
  unsigned int m_firstGroundID;
  unsigned int m_lastGroundID;
  unsigned int m_previousGroundID;
  int m_counterGroundID;

  // Datagrams:
  int m_counterDataDiagramsEpu0;
  int m_nbrDataGramsEpu0;
  int m_nbrEventsDataGramsEpu0;
  unsigned int m_firstDataGramEpu0;
  unsigned int m_thisDataGramEpu0;
  unsigned int m_previousDataGramEpu0;
  unsigned int m_previousPreviousDataGramEpu0;
  int m_endRunDataGramEpu0;
  int m_fullDataGramEpu0;
  int m_beginRunDataGramEpu0;

  int m_counterDataDiagramsEpu1;
  int m_nbrDataGramsEpu1;
  int m_nbrEventsDataGramsEpu1;
  unsigned int m_firstDataGramEpu1;
  unsigned int m_thisDataGramEpu1;
  unsigned int m_previousDataGramEpu1;
  unsigned int m_previousPreviousDataGramEpu1;
  int m_endRunDataGramEpu1;
  int m_fullDataGramEpu1;
  int m_beginRunDataGramEpu1;

  int m_counterDataDiagramsEpu2;
  int m_nbrDataGramsEpu2;
  int m_nbrEventsDataGramsEpu2;
  unsigned int m_firstDataGramEpu2;
  unsigned int m_thisDataGramEpu2;
  unsigned int m_previousDataGramEpu2;
  unsigned int m_previousPreviousDataGramEpu2;
  int m_endRunDataGramEpu2;
  int m_fullDataGramEpu2;
  int m_beginRunDataGramEpu2;

  int m_counterCyclesSiu0;
  int m_counterDataDiagramsSiu0;
  int m_nbrDataGramsSiu0;
  int m_nbrEventsDataGramsSiu0;
  unsigned int m_firstDataGramSiu0;
  unsigned int m_thisDataGramSiu0;
  unsigned int m_previousDataGramSiu0;
  unsigned int m_previousPreviousDataGramSiu0;
  int m_endCountDataGramSiu0;
  int m_fullDataGramSiu0;
  int m_beginRunDataGramSiu0;

  int m_counterCyclesSiu1;
  int m_counterDataDiagramsSiu1;
  int m_nbrDataGramsSiu1;
  int m_nbrEventsDataGramsSiu1;
  unsigned int m_firstDataGramSiu1;
  unsigned int m_thisDataGramSiu1;
  unsigned int m_previousDataGramSiu1;
  unsigned int m_previousPreviousDataGramSiu1;
  int m_endCountDataGramSiu1;
  int m_fullDataGramSiu1;
  int m_beginRunDataGramSiu1;

  int m_datagramGapsEPU0;
  int m_datagramGapsEPU1;
  int m_datagramGapsEPU2;
  int m_datagramGapsSIU0;
  int m_datagramGapsSIU1;

  /// number of events in the digi root file
  int m_nEvent;
  int m_nEventNoPeriodic;

  // Number of GEM related quantities:
  ULong64_t m_nbrPrescaled;
  ULong64_t m_nbrDeadZone;
  ULong64_t m_nbrDiscarded;
  Long64_t m_deltaSequenceNbrEvents;

  /// number of events with 3 in a row trigger in GEM
  int m_nTkrTrigger;

  /// number of events with strip ID outside the range from 0 to 1535
  int m_nEventBadStrip;

  /// number of events with more than 63 strips per GTRC
  int m_nEventMoreStrip;

  /// number of events with saturated TOT (250 ADC )
  int m_nEventSatTot;

  /// number of events with 0 TOT but at least 1 strip
  int m_nEventZeroTot;

  /// number of events with TOT values outside range [0, g_overlapTot]
  int m_nEvtInvalidTot;

  /// number of events with TOT values outside range [0, g_satTot] i.e. overlapped triggers
  int m_nEvtOverlapTriggerTot;

  /// number of events with none zero TOT but no strip hit
  int m_nEventBadTot;

  /// time of first trigger
  UInt_t m_startTime;

  /// time of last trigger
  UInt_t m_endTime;

  // Livetime:
  double m_liveTime;
 
  ULong64_t m_elapsedTime;

  int m_nbrEventsNormal;
  int m_nbrEvents4Range;
  int m_nbrEvents4RangeNonZS;

  // Extended counters problem?
  Int_t m_extendedCountersFlag;

  // Time tone problem?
  Int_t m_backwardsTimeTone;
  Int_t m_identicalTimeTones;

  // Timetone counters and flags:
  UInt_t m_nbrFlywheeling;

  Int_t m_nbrIncomplete;
  Int_t m_nbrMissingGps;
  Int_t m_nbrMissingCpuPps;
  Int_t m_nbrMissingLatPps;
  Int_t m_nbrMissingTimeTone;

  /// percentage of events with TKR trigger but less than 6 digis in a tower
  TGraph* m_nDigi;

  // ACD parity errors:
  unsigned int m_nAcdOddParityError;
  unsigned int m_nAcdHeaderParityError;


  // Error flags:
  Int_t m_eventBadEventSequence;
  Int_t m_eventBadTkrRecon;
  Int_t m_eventPacketError;
  Int_t m_eventTemError;
  Int_t m_eventTrgParityError;
  Int_t m_eventBadLdfStatus;
  Int_t m_eventGtrcPhase;
  Int_t m_eventGtfePhase;
  Int_t m_eventGtccFifo;
  Int_t m_eventGtccHdrParity;
  Int_t m_eventGtccWcParity;
  Int_t m_eventGtrcSummary;
  Int_t m_eventGtccDataParity;
  Int_t m_eventGtccTimeout;
  Int_t m_eventGcccError;
  Int_t m_eventGtccError;
  Int_t m_eventPhaseError;
  Int_t m_eventTimeoutError;

  Int_t m_eventIsPeriodic;

  // Acd Digi based histograms
  TH1F* m_AcdTileIdOnePMT;
  TH1F* m_AcdTileIdOneVeto;

  TH1F* m_AcdHitMap;
  TH1F* m_AcdVetoMap;
  
  TH2F* m_AcdPhaMapA;
  TH2F* m_AcdPhaMapB;

  // Acd Recon based histograms
  TH1F* m_AcdEfficMap;
  TH1F* m_AcdInEfficMap;

  TH2F* m_AcdMissMapTop;
  TH2F* m_AcdMissMapMinusX;
  TH2F* m_AcdMissMapMinusY;
  TH2F* m_AcdMissMapPlusX;
  TH2F* m_AcdMissMapPlusY;



  // Triggger rates:
  TH1F* m_triggerRate;
  TH1F* m_triggerLivetimeRate;
  TH1F* m_livetimeRate;
  TH1F* m_deadzoneRate;
  TH1F* m_discardedRate;


  /// number of events with different Glt trigger:
  long m_nEvtGltTrigger[enums::number_of_trigger_bits];

  /// condition summary in GEM
  TH1F* m_condSummary;


  /// number of events with different GEM trigger
  long m_nEvtGemTrigger[enums::GEM_offset];


  /// number of events with different number of digis.
  /// For example: m_nEventDigi[0] is number of events with 0 digi
  /// m_nEventDigi[6] is number of events with >= 6 digis
  /// If there are more than one tower, maximal number of digis in any single
  /// tower is used
  int m_nEventDigi[7];


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



  // epu number
  TH1F* m_epu;

  TH1F* m_datagramsEPU0;
  TH1F* m_datagramsEPU1;
  TH1F* m_datagramsEPU2;
  TH1F* m_datagramsSIU0;
  TH1F* m_datagramsSIU1;

  TH1F* m_deltaTimeDGCTEvtEPU0;
  TH1F* m_deltaTimeDGCTEvtEPU1;
  TH1F* m_deltaTimeDGCTEvtEPU2;
  TH1F* m_deltaTimeDGCTEvtSIU0;
  TH1F* m_deltaTimeDGCTEvtSIU1;

  TH1F* m_deltaEventIDEPU0;
  TH1F* m_deltaEventIDEPU1;
  TH1F* m_deltaEventIDEPU2;
  TH1F* m_deltaEventIDSIU0;
  TH1F* m_deltaEventIDSIU1;

  // GEM discarded events:
  TH1F* m_gemDiscarded;
  TH1F* m_gemDiscardedTime;

  // GEM deadzone events:
  TH1F* m_gemDeadzone;

  // Number of triggers per tower:
  TH1F* m_tkrPerTower;
  TH1F* m_calLoPerTower;
  TH1F* m_calHiPerTower;

  int m_tkrPerTowerArray[g_nTower];
  int m_calLoPerTowerArray[g_nTower];
  int m_calHiPerTowerArray[g_nTower];

  /// histogram of time between adjacent event in mili second
  TH1F* m_timeInterval;

  /// histogram of time between adjacent event in mili second with a cut of
  /// 3 ms to study the dead time
  TH1F* m_timeIntervalCut;

  /// histogram of time between adjacent event in mili second, taken from GEM
  TH1F* m_timeIntervalGem;
  TH1F* m_timeIntervalGemZoom;

  TH1F* m_deltaWindowOpenTime;
  TH1F* m_deltaWindowOpenTimeZoom;

  TH1F* m_timeIntervalElapsed;

  TH1F* m_condArrivalTimeTKR;
  TH1F* m_condArrivalTimeROI;
  TH1F* m_condArrivalTimeCalLo;
  TH1F* m_condArrivalTimeCalHi;
  TH1F* m_condArrivalTimeCNO;


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

  // Histograms for the ACD digis:
  TH1F* m_nAcdDigis;


  TH2F* m_AcdGarcGafeHitMap;
  TH2F* m_AcdGarcGafeVetoMap;  

  // Acd-related stuff in the GEM
  TH1F* m_AcdGemVetoMap;
  TH1F* m_AcdGemCnoMap;
  TH1F* m_AcdGemRoiMap;

  // Path-length corrected MIP
  TH2F* m_AcdMipMapA;
  TH2F* m_AcdMipMapB;


};
#endif
