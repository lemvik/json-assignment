#include "utils.h"

namespace json {
  namespace utils {
    
    template<>
    std::string to_string<bool>(const bool& val) {
      const std::string t = "true";
      const std::string f = "false";
      return val ? t : f;
    }
  }
}
