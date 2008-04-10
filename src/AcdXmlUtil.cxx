#ifndef AcdXmlUtil_cxx
#define AcdXmlUtil_cxx
#include "AcdXmlUtil.h"
#include "DomElement.h"

#include "xmlBase/Dom.h"
#include "xmlBase/XmlParser.h"

#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/util/XercesDefs.hpp"
#include <xercesc/util/XMLString.hpp>

using std::cout;
using std::endl;

DomElement
AcdXmlUtil::makeDocument(const char* name){

  using XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation;
  using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
  using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

  // parser not actually used, but needed to initialize XMLPlatformUtils properly
  xmlBase::XmlParser parser; 
  XMLCh tempStr[100];
  XMLString::transcode(name,tempStr,99);
  DOMElement* newChild = DOMImplementation::getImplementation()->createDocument()->createElement(tempStr);
  newChild->getOwnerDocument()->appendChild(newChild);
  return DomElement(*newChild);
}

DomElement 
AcdXmlUtil::makeChildNode(DomElement& domNode, const char* name) {

  using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
  using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

  XMLCh tempStr[100];
  XMLString::transcode(name,tempStr,99);
  DOMElement* newChild = domNode().getOwnerDocument()->createElement(tempStr);
  domNode().appendChild(newChild);
  return DomElement(*newChild);
}

void  
AcdXmlUtil::addAttribute(DomElement& domNode, const char* name, const char* val) {
  xmlBase::Dom::addAttribute(&(domNode()),std::string(name),val);
}

void  
AcdXmlUtil::addAttribute(DomElement& domNode, const char* name, const int& val) {
  xmlBase::Dom::addAttribute(&(domNode()),std::string(name),val);
}

void  
AcdXmlUtil::addAttribute(DomElement& domNode, const char* name, const double& val) {
  xmlBase::Dom::addAttribute(&(domNode()),std::string(name),val);
}

bool  
AcdXmlUtil::writeIt(DomElement& doc,const char* fileName) {
  return xmlBase::Dom::writeIt(doc().getOwnerDocument(),fileName);
}


#endif
