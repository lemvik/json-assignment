#include "json.h"
#include "json_parser.h"
#include "json_sa.h"

#include <stack>
#include <sstream>
#include <iostream>

namespace json {
  namespace parser {

    enum class next_token : int { none = 0, value = 1<<0, comma = 1<<1, colon = 1<<2, key = 1<<3 };

    inline next_token operator | (next_token l, next_token r) {
      using type = std::underlying_type_t<next_token>;
      return static_cast<next_token>(static_cast<type>(l) | static_cast<type>(r));
    }

    inline next_token operator & (next_token l, next_token r) {
      using type = std::underlying_type_t<next_token>;
      return static_cast<next_token>(static_cast<type>(l) & static_cast<type>(r));
    }

    template<typename T>
    std::string to_string(const T& val) {
      std::stringstream ss;
      ss << val;
      return ss.str();
    }

    class builder_callback : public simple::token_callback {
      bool failed;
      value root;                             // By default we will have null value.
      std::stack<next_token> context;
      std::stack<value*> objects_being_built; // Stack of non-owning pointers, for in-place building.
      std::stack<std::string> keys;

    public:
      builder_callback() : failed(false), root(), context(), objects_being_built(), keys() {}

      void json_start() override {
        context.push(next_token::value);
      }

      void json_end() override {}

      void json_string(const std::string& str) override {
        if (expects(next_token::value)) {
          context.pop();
          attach(value(str));
          value_read();
        } else if (expects(next_token::key)) {
          context.pop();
          context.push(next_token::colon);
          keys.push(str);
        } else {
          fail("[string:" + str + "]");
        }
      }

      void json_number(double num) override {
        if (expects(next_token::value)) {
          context.pop();
          attach(value(num));
          value_read();
        } else {
          fail("[double:" + to_string(num) + "]");
        }
      }

      void json_boolean(bool flag) override {
        if (expects(next_token::value)) {
          context.pop();
          attach(value(flag));
          value_read();
        } else {
          fail("[boolean:" + to_string(flag ? "true" : "false") + "]");
        }
      }

      void json_null() override {
        if (expects(next_token::value)) {
          context.pop();
          attach(value());
          value_read();
        } else {
          fail("[null]");
        }
      }

      void json_comma() override {
        if (expects(next_token::comma)) {
          context.pop();
          if (in_array()) {
            context.push(next_token::value);
          } else if (in_object()) {
            context.push(next_token::key);
          }
        } else {
          fail("[,(comma)]");
        }
      }

      void json_colon() override {
        if (expects(next_token::colon)) {
          context.pop();
          context.push(next_token::value);
        } else {
          fail("[:(colon)]");
        }
      }

      void json_array_starts() override {
        context.push(next_token::value);
        objects_being_built.push(attach(value(value_type::array)));
      }

      void json_array_ends() override {
        if (expects(next_token::comma)) {
          context.pop(); // Pop the comma 
          context.pop(); // Pop the value
          objects_being_built.pop();
          value_read();
        } else {
          fail("[(value)]");
        }
      }

      void json_object_starts() override {
        context.push(next_token::key);
        objects_being_built.push(attach(value(value_type::object)));
      }

      void json_object_ends() override {
        if (expects(next_token::comma)) {
          context.pop(); // Pop the comma 
          context.pop(); // Pop the value
          objects_being_built.pop();
          value_read();
        } else {
          fail("[(key)]");
        }
      }

      void json_error(const std::string& error) override {
        throw json::json_error("Encountered an error during parse: " + error);
      }

      virtual bool need_more_json() override { return !failed && !expects(next_token::none); }

      value result() {
        if (!need_more_json()) {
          return root; 
        }
        throw json::json_error("Parsing process is not finished: [built=" + to_string(objects_being_built.size()) + "][need_more=" + to_string(need_more_json() ? "true" : "false") + "]");
      }

    private:
      bool expects(const next_token& val) const {
        if (context.empty()) {
          return val == next_token::none;
        }

        return (context.top() & val) != next_token::none;
      }

      bool in_array() const {
        if (objects_being_built.empty()) {
          return false;
        }
        auto& current_object = objects_being_built.top();
        return current_object && current_object->get_type() == value_type::array;
      }

      bool in_object() const {
        if (objects_being_built.empty()) {
          return false;
        }
        auto& current_object = objects_being_built.top();
        return current_object && current_object->get_type() == value_type::object;
      }

      value* attach(value&& value) {
        if (objects_being_built.empty()) {
          root = std::move(value);
          return &root;
        }

        auto& current_object = objects_being_built.top();
        switch(current_object->get_type()) {
        case value_type::object: {
          std::string key = keys.top();
          (*current_object)[key] = std::move(value);
          keys.pop();
          return &((*current_object)[key]);
        }
        case value_type::array: {
          size_t last_index = (*current_object).push(std::move(value));
          return &((*current_object)[last_index]);
        }
        default:
          throw json::json_error("Cannot attach to a [type=" + to_string(current_object->get_type()) + "] value");
        }
      }

      void value_read() {
        if (in_array() || in_object()) {
          context.push(next_token::comma);
        }
      }

      void fail(const std::string& unnext_token) {
        failed = true;
        std::cerr << "Next_Token to see " << next_token_values() << ", but got " << unnext_token << "\n";
      }

      std::string next_token_values() const {
        std::stringstream ss;
        if (expects(next_token::value)) {
          ss << "[string, null, true, false, number]";
        }

        if (expects(next_token::comma)) {
          ss << "[,(comma)]";
        }

        if (expects(next_token::colon)) {
          ss << "[:(colon)]";
        }

        if (expects(next_token::key)) {
          ss << "[:(colon)]";
        }
        
        return ss.str();
      }
    };

    json::value parse(const std::string& source) {
      builder_callback callback;
      run_tokenizer(source, callback);
      return callback.result();
    }

    json::value parse(std::istream& source) {
      builder_callback callback;
      run_tokenizer(source, callback);
      return callback.result();
    }
  }
}
