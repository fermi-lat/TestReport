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

class IntervalError{
 friend class ft2Verify;
 public:
  IntervalError(string errName, float errValue);
  ~IntervalError();

 private:
  string m_errName;
  float m_errValue;
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
  // write interval summary to xml file
  void writeXmlIntervalSummary(DomElement& node, int truncation) const;

 private:
  map< string, list<int> > m_errMap;
  map< int, list<IntervalError*> > m_intMap;
};

#endif
