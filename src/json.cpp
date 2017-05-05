#include "json.h"

namespace json {

  std::ostream& operator<<(std::ostream& os, const json_value_type& val) {
    switch (val) {
    case json_value_type::null: os << "null"; break;
    case json_value_type::number: os << "number"; break;
    case json_value_type::boolean: os << "boolean"; break;
    case json_value_type::string: os << "string"; break;
    case json_value_type::object: os << "object"; break;
    case json_value_type::array: os << "array"; break;
    }
    return os;
  }
}
