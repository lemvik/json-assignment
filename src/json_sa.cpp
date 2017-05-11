#include "json_sa.h"

#include <cctype>
#include <memory>
#include <vector>
#include <sstream>
#include <istream>
#include <iostream>

namespace json {
  namespace simple {

    bool literal(std::istream& is, const std::string& literal) {
      auto len = literal.length();
      std::vector<std::string::value_type> read_chars(len);

      unsigned int i = 0;
      while (is && i < len) {
        std::string::value_type c;
        is >> c;
        if (c != literal[i]) {
          break;
        }
        read_chars[i] = c;
        ++i;
      }

      // If we failed to read enough, but read something - put back the rest
      if (i < len && is && i != 0) {
        for (auto j = i - 1; j != 0; --j) {
          is.putback(read_chars[j]);
        }
      }

      return is && i == len;
    }

    bool read_string(std::istream& is, std::string& target) {
      std::stringstream buffer;
      std::string::value_type c;

      bool escaped = true;
      std::ios::fmtflags in_flags(is.flags());
      is >> std::noskipws;
      while (is) {
        is >> c;
        if (!is) {
          is.flags(in_flags);
          return false;
        }

        if (c == '"' && !escaped) {
          target = buffer.str();
          is.flags(in_flags);
          return true;
        }

        escaped = (c == '\\' && !escaped);
        buffer.put(c);
      }
      
      is.flags(in_flags);
      return !is.fail();
    }

    bool read_number(std::istream& is, double& val) {
      is >> val;
      return !is.fail();
    }

    void run_tokenizer(const std::string& source, token_callback& callback) {
      if (source.empty()) {
        callback.json_error("Cannot parse an empty string, top level value in JSON should be one of 'true', 'false', 'null', string literal, object or an array.");
        return;
      }

      std::stringstream is(source);

      run_tokenizer(is, callback);
    }

    void run_tokenizer(std::istream& is, token_callback& callback) {
      callback.json_start();

      while (is && callback.need_more_json()) {
        is >> std::ws;
        auto c = is.peek();
        if (!std::stringstream::traits_type::not_eof(c)) {
          callback.json_error("Unable to proceed with reading: unable to read char.");
          return;
        }
        
        switch (c) {
        case 'n':
          if (literal(is, "null")) {
            callback.json_null();
            continue;
          } else {
            callback.json_error("Expected to read `null`, but failed.");
            return;
          }
        case 't':
          if (literal(is, "true")) {
            callback.json_boolean(true);
            continue;
          } else {
            callback.json_error("Expected to read `true`, but failed.");
            return;
          }
        case 'f':
          if (literal(is, "false")) {
            callback.json_boolean(false);
            continue;
          } else {
            callback.json_error("Expected to read `false`, but failed.");
            return;
          }
        case '{':
          is.get();
          callback.json_object_starts();
          continue;
        case '}':
          is.get();
          callback.json_object_ends();
          continue;
        case '[':
          is.get();
          callback.json_array_starts();
          continue;
        case ']':
          is.get();
          callback.json_array_ends();
          continue;
        case ':':
          is.get();
          callback.json_colon();
          continue;
        case ',':
          is.get();
          callback.json_comma();
          continue;
        case '"': {
          is.get();
          std::string str;
          if (read_string(is, str)) {
            callback.json_string(str);
            continue;
          } else {
            callback.json_error("Expected to read JSON string, but failed.");
            return;
          }
        }
        default:
          if (c == '-' || std::isdigit(c)) {
            double value;
            if (read_number(is, value)) {
              callback.json_number(value);
              continue;
            } else {
              callback.json_error("Expected to read JSON number, but failed.");
              return;
            }
          } else {
            callback.json_error("Unknown character was read: " + std::string(1, std::stringstream::traits_type::to_char_type(c)) + ", terminating.");
            return;
          } 
        }
      }

      callback.json_end();
    }
  }
}
