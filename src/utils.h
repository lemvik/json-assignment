#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <sstream>

namespace json {
  namespace utils {
    template<typename T>
    std::string to_string(const T& obj) {
      std::stringstream ss;
      ss << obj;
      return ss.str();
    }

    template<>
    std::string to_string<bool>(const bool& val) {
      const std::string t = "true";
      const std::string f = "false";
      return val ? t : f;
    }
  }
}

#endif 
