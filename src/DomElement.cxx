#ifndef DomElement_cxx
#define DomElement_cxx

#include "DomElement.h"

#include "xercesc/dom/DOMElement.hpp"

DomElement::DomElement(DOMElement& elem)
  :_elem(&elem){;}


DomElement& 
DomElement::operator=(const DomElement& other){
  _elem = other._elem;
  return *this;
}


#endif
