#ifndef ToString_Function
#define ToString_Function

#include <sstream>
#include <string>

template<class X> std::string ToString(X data, int precision=2)
{
  std::ostringstream stream;
  stream.precision(precision);
  stream << data;
  return stream.str();
}

#endif
