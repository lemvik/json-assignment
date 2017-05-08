#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "json.h"
#include <memory>
#include <string>

namespace json {
  namespace parser {
    value parse(const std::string&);
  }
}

#endif
