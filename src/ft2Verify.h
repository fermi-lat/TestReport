#ifndef ft2Verify_Header
#define ft2Verify_Header

#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <xercesc/util/XercesDefs.hpp>
#include "xercesc/dom/DOMElement.hpp"

/**
 * \class ft2Verify
 * \brief ft2Verify verification of digi data
 * \author M.E.
 */

using std::list;
using std::string;
using std::vector;
using std::map;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class DomElement;

class RowError{
 friend class ft2Verify;
 public:
  RowError(string errName, double livetime, double tstart, double tstop);
  ~RowError();

 private:
  string m_errName;
  double m_livetime;
  double m_tstart;
  double m_tstop;
};

class ft2Verify{
 public:
  ft2Verify();
  ~ft2Verify();

  void analyzeFt2(const char* ft2FileName);
  
  // write errors to an xml file
  bool writeXmlFile(const char* fileName, int truncation) const;
  // write xml header
  void writeXmlHeader(DomElement& node) const;
  // write xml footer
  void writeXmlFooter(DomElement& node) const;
  // write error summary to xml file
  void writeXmlErrorSummary(DomElement& node, int truncation) const;
  // write row summary to xml file
  void writeXmlRowSummary(DomElement& node, int truncation) const;

 private:
  int m_nRows;
  map< string, list<int> > m_errMap;
  map< int, list<RowError*> > m_rowMap;
};

#endif
