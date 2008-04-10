#ifndef AcdXmlUtil_h
#define AcdXmlUtil_h 

class DomElement;

/** 
 *
 * @brief A few function to help manage xml
 *
 */

namespace AcdXmlUtil {

  DomElement makeDocument(const char* topNode);
  DomElement makeChildNode(DomElement& domNode, const char* name);
  
  void addAttribute(DomElement& domNode, const char* name, const char* val);
  void addAttribute(DomElement& domNode, const char* name, const int& val);
  void addAttribute(DomElement& domNode, const char* name, const double& val);
  
  bool writeIt(DomElement& doc, const char* fileName);

};

#endif
