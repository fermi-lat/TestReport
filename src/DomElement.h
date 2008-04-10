#ifndef DomElement_h
#define DomElement_h 

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class  DOMElement;
XERCES_CPP_NAMESPACE_END

using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

/** 
 * @class DomElement
 *
 * @brief wrapper for DOMElement, Needed to protect CINT (i think)
 *
 * @author Eric Charles
 * $Header$
 */


class DomElement {
  
public:
  DomElement(DOMElement& elem);
  ~DomElement(){}
  DomElement& operator=(const DomElement& other);
  DOMElement& operator()(){
    return *_elem;
  }  
  const DOMElement& operator()() const {
    return *_elem;
  }
private:
  DOMElement* _elem;
};

#endif
