#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "json.h"
#include <string>
#include <istream>

namespace json {
  namespace parser {
    // Parses given string and returns first fully parsed value. If anything goes wrong,
    // throws json::json_error
    value parse(const std::string&);
    // Parses given stream and returns first fully parsed value. If anything goes wrong,
    // throws json::json_error
    value parse(std::istream&);
  }
}

#endif
