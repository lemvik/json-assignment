#include "json_sa.h"
#include "utils.h"

#include <cctype>
#include <memory>
#include <vector>
#include <sstream>
#include <istream>
#include <iostream>
#include <stdexcept>

namespace json {
  namespace simple {

    // Parser error - not visible to outside world, used to convey information about 
    // parsing failure.
    struct parser_error : public std::runtime_error {
      parser_error(const std::string reason) : std::runtime_error(reason) {}
    };

    // Tries to consume given literal from given stream.
    // Since literal is known at call time and might not correspond to any meaningful value (ex. in case of null) doesn't
    // return anything. 
    void read_literal(std::istream& is, const std::string& literal) {
      auto len = literal.length();

      unsigned int i = 0;
      while (is && i < len) {
        if (is.get() != literal[i]) {
          break;
        }
        ++i;
      }

      if (!is || i != len) {
        throw parser_error("Failed to read [literal=" + literal + "][stream_state=" + utils::to_string((bool)is) + "][read=" + utils::to_string(i) + "]");
      }
    }

    // Reads JSON string from given stream. Assumes that JSON string terminates with first
    // non-escaped '"' (another way would be to check that first char is '"' and pass whole string here,
    // but it looks superfluous).
    std::string read_string(std::istream& is) {
      std::string target;
      std::string::value_type c;

      bool escaped = true;
      std::ios::fmtflags in_flags(is.flags());
      is >> std::noskipws;
      while (is) {
        is >> c;
        if (!is) {
          is.flags(in_flags);
          throw parser_error("Failed to read string due to a stream error.");
        }

        if (c == '"' && !escaped) {
          is.flags(in_flags);
          return target;
        }

        escaped = (c == '\\' && !escaped);
        target.push_back(c);
      }
      
      is.flags(in_flags);

      throw parser_error("Failed to read string: unterminated string encountered.");
    }

    // Reads JSON number. As per JSON spec (json.org) numbers should have . as their decimal
    // separators. One way to ensure that would be to enforce locale on input streams, but it might
    // have averse effects on characters reading so I went for bit uglier, but safe approach here.
    double read_number(std::istream& is) {
      const auto classic_locale = std::locale::classic();
      auto original_locale = is.imbue(classic_locale); // Imbue with C locale to force decimal separator to . (some of locales use ,)
      double val;
      is >> val;
      is.imbue(original_locale);
      if (!is) {
        throw parser_error("Failed to read number due to a stream error.");
      }
      return val; 
    }

    // Docs in header.
    void run_tokenizer(const std::string& source, token_callback& callback) {
      if (source.empty()) {
        callback.json_error("Cannot parse an empty string, top level value in JSON should be one of 'true', 'false', 'null', a string literal, an object or an array.");
        return;
      }

      std::stringstream is(source);

      run_tokenizer(is, callback);
    }

    // Docs in header.
    void run_tokenizer(std::istream& is, token_callback& callback) {
      const std::string null_literal = "null";
      const std::string true_literal = "true";
      const std::string false_literal = "false";

      if (!is) {
        callback.json_error("Unable to proceed with reading: given stream is broken.");
        return;
      }

      callback.json_start();

      while (is && callback.need_more_json()) {
        is >> std::ws;
        if (!is) {
          callback.json_error("Unable to proceed with reading: skip whitespaces for some inexplicable reason.");
          return;
        }
        auto c = is.peek();
        if (!is) {
          callback.json_error("Unable to proceed with reading: unable to read char.");
          return;
        }
        
        switch (c) {
        case 'n':
          try {
            read_literal(is, null_literal);
            callback.json_null();
            break;
          } catch (const parser_error& error) {
            callback.json_error(error.what());
            return; // Return here and below because we cannot rely on callback.neet_more_json() to return false
                    // even after error and is could be readable
          }
        case 't':
          try {
            read_literal(is, true_literal);
            callback.json_boolean(true);
            break;
          } catch (const parser_error& error) {
            callback.json_error(error.what());
            return;
          }
        case 'f':
          try {
            read_literal(is, false_literal);
            callback.json_boolean(false);
            break;
          } catch (const parser_error& error) {
            callback.json_error(error.what());
            return;
          }
        case '{':
          is.get();
          callback.json_object_starts();
          break;
        case '}':
          is.get();
          callback.json_object_ends();
          break;
        case '[':
          is.get();
          callback.json_array_starts();
          break;
        case ']':
          is.get();
          callback.json_array_ends();
          break;
        case ':':
          is.get();
          callback.json_colon();
          break;
        case ',':
          is.get();
          callback.json_comma();
          break;
        case '"': 
          is.get(); // Drop the '"'
          try {
            callback.json_string(read_string(is));
            break;
          } catch (const parser_error& error) {
            callback.json_error(error.what());
            return;
          }
        default:
          if (c == '-' || std::isdigit(c)) {
            try {
              callback.json_number(read_number(is));
              break;
            } catch (const parser_error& error) {
              callback.json_error(error.what());
              return;
            }
          } else {
            callback.json_error("Unknown character was read: '" + utils::to_string(c) + "', terminating.");
            return;
          } 
        }
      }

      // Given code structure, this call is likely unneeded, but can be used to do some finalization
      callback.json_end();
    }
  }
}
