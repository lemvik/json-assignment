#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "json.h"
#include <memory>
#include <string>

namespace json {
  namespace parser {
    std::unique_ptr<json_node> parse(const std::string&);
  }
}

#endif
