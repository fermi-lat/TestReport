#ifndef meritVerify_Header
#define meritVerify_Header

#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <xercesc/util/XercesDefs.hpp>
#include "xercesc/dom/DOMElement.hpp"

/**
 * \class meritVerify
 * \brief meritVerify verification of merit data
 * \author M.E.
 */

using std::list;
using std::string;
using std::vector;
using std::map;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class DomElement;

class RowError{
 friend class meritVerify;
 public:
  RowError(string errName, double eventid, double time);
  ~RowError();

 private:
  string m_errName;
  double m_eventid;
  double m_time;
};

class meritVerify{
 public:
  meritVerify();
  ~meritVerify();

  void analyzeMerit(const char* meritFileName);
  
  // write errors to an xml file
  bool writeXmlFile(const char* fileName, int truncation) const;
  // write xml header
  void writeXmlHeader(DomElement& node) const;
  // write xml footer
  void writeXmlFooter(DomElement& node) const;
  // write error summary to xml file
  void writeXmlErrorSummary(DomElement& node, int truncation) const;
  // write event summary to xml file
  void writeXmlEventSummary(DomElement& node, int truncation) const;

 private:
  int m_nRows;
  map< string, list<int> > m_errMap;
  map< int, list<RowError*> > m_rowMap;
};

#endif
