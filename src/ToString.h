#ifndef ToString_Function
#define ToString_Function

#include <sstream>
#include <string>

template<class X> std::string ToString(X data)
{
  std::ostringstream stream;
  stream << data;
  return stream.str();
}

#endif
