#include <iostream>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <iterator>
#include <list>
#include <algorithm>

#include "ft2Verify.h"
#include "AcdXmlUtil.h"
#include "DomElement.h"
#include "TSystem.h"

#include "tip/Header.h"
#include "tip/IFileSvc.h"
#include "tip/Table.h"
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

ft2Verify::ft2Verify()
  : m_nRows(0)
{ 
}

ft2Verify::~ft2Verify()
{
  m_errMap.clear();
  m_rowMap.clear();
}

RowError::RowError(string errName, double livetime, double tstart, double tstop)
  : m_errName(errName),
    m_livetime(livetime),
    m_tstart(tstart),
    m_tstop(tstop)
{
}

RowError::~RowError() {
}

void ft2Verify::analyzeFt2(const char* ft2FileName="ft2.fit")
{
  int rowCtr = 0;
  double tdiff, tstart, tstop, livetime;
  std::string errorName;

  //open fits file using the readTable method
  const tip::Table* ft2file = tip::IFileSvc::instance().readTable(ft2FileName, "SC_DATA");
  tip::Table::ConstIterator it = ft2file->begin();
  tip::Table::ConstRecord& row = *it;

  for ( ; it != ft2file->end(); ++it) {
    rowCtr++;
    tstart = row["start"].get();
    tstop = row["stop"].get();
    livetime = row["livetime"].get();
    tdiff = (tstop - tstart);

    if (livetime < 0) {
      cout << "ERROR! Negative livetime at row " << rowCtr << endl;
      errorName = "FT2_NEGATIVE_LIVETIME"; // ['Livetime is negative for this row']
      RowError* row_e = new RowError(errorName,livetime,tstart,tstop);
      m_rowMap[rowCtr].push_back(row_e);
      m_errMap[errorName].push_back(rowCtr);
    }
    if (livetime > tdiff) {
      cout << "ERROR! Livetime is bigger than elapsed time at inteval " << rowCtr << endl;
      errorName = "FT2_LIVETIME_UNPHYSICAL"; // ['Livetime is greater than elapsed time for this row']
      RowError* row_e = new RowError(errorName,livetime,tstart,tstop);
      m_rowMap[rowCtr].push_back(row_e);
      m_errMap[errorName].push_back(rowCtr);
    }
  }
  m_nRows = rowCtr;
  // Closing time 
  delete ft2file;
}

Bool_t ft2Verify::writeXmlFile(const char* fileName, int truncated) const {

  DomElement elem = AcdXmlUtil::makeDocument("errorContribution");
  writeXmlHeader(elem);
  writeXmlErrorSummary(elem, truncated);
  writeXmlEventSummary(elem, truncated);
  writeXmlFooter(elem);
  return AcdXmlUtil::writeIt(elem,fileName);

}

void ft2Verify::writeXmlHeader(DomElement& /* node */) const {
  // do nothing, for now
}

void ft2Verify::writeXmlFooter(DomElement& /* node */) const {
  // do nothing, for now
  return;
}

void ft2Verify::writeXmlErrorSummary(DomElement& node, int truncation) const {
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

void ft2Verify::writeXmlEventSummary(DomElement& node, int truncation) const {
  DomElement eventSummary = AcdXmlUtil::makeChildNode(node,"eventSummary");
  AcdXmlUtil::addAttribute(eventSummary,"num_processed_events",m_nRows);
  AcdXmlUtil::addAttribute(eventSummary,"num_error_events",(int)m_rowMap.size());
  map< string, int > m_errCounter;
  for (map< string, list<int> >::const_iterator it = m_errMap.begin(); it != m_errMap.end(); it++){
    m_errCounter[(*it).first.c_str()] = 0;
    if ((int)(*it).second.size()>truncation) AcdXmlUtil::addAttribute(eventSummary,"truncated","True");
    else AcdXmlUtil::addAttribute(eventSummary,"truncated","False");
  }
  for (map< int, list<RowError*> >::const_iterator it = m_rowMap.begin(); it != m_rowMap.end(); it++){  
    // check if there is at least one error that didn't reach truncation limit for this event
    bool write_row = false;
    for (list<RowError*>::const_iterator ie = (*it).second.begin(); ie != (*it).second.end(); ie ++){
      if (m_errCounter[(*ie)->m_errName.c_str()] <= truncation) write_row = true;
     }
    if (write_row) {
      DomElement eventError = AcdXmlUtil::makeChildNode(eventSummary,"errorEvent");
      if ( (*it).first > -1 ) AcdXmlUtil::addAttribute(eventError,"rowNumber",(int)(*it).first);
      else AcdXmlUtil::addAttribute(eventError,"rowNumber","noRow");
      for (list<RowError*>::const_iterator ie = (*it).second.begin(); ie != (*it).second.end(); ie ++){
        DomElement errDetail = AcdXmlUtil::makeChildNode(eventError,"error");
        AcdXmlUtil::addAttribute(errDetail,"code",(*ie)->m_errName.c_str());
        AcdXmlUtil::addAttribute(errDetail,"livetime",(*ie)->m_livetime);
        AcdXmlUtil::addAttribute(errDetail,"tstart",(int)(*ie)->m_tstart);
        AcdXmlUtil::addAttribute(errDetail,"tstop",(int)(*ie)->m_tstop);
        m_errCounter[(*ie)->m_errName.c_str()]++;
      }
    }
  }
}

