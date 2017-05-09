#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "json.h"
#include <memory>
#include <string>
#include <istream>

namespace json {
  namespace parser {
    value parse(const std::string&);
    value parse(std::istream&);
  }
}

#endif
