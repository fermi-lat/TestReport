#ifndef ToString_Function
#define ToString_Function

#include <sstream>
#include <string>

template<class X> std::string& ToString(X data)
{
  std::stringstream stream;
  static std::string s;
  stream << data;
  stream >> s;
  return s;
}

#endif
