#include "json_parser.h"
#include "json_sa.h"

#include <cassert>
#include <stack>
#include <sstream>
#include <iostream>
// In C++17 just remove experimental.
#include <experimental/optional>

namespace json {
  namespace parser {

    enum class expected : int { none = 0, value = 1<<0, comma = 1<<1, colon = 1<<2, key = 1<<3 };

    inline expected operator | (expected l, expected r) {
      using type = std::underlying_type_t<expected>;
      return static_cast<expected>(static_cast<type>(l) | static_cast<type>(r));
    }

    inline expected operator & (expected l, expected r) {
      using type = std::underlying_type_t<expected>;
      return static_cast<expected>(static_cast<type>(l) & static_cast<type>(r));
    }

    template<typename T>
    std::string to_string(const T& val) {
      std::stringstream ss;
      ss << val;
      return ss.str();
    }

    class builder_callback : public simple::token_callback {
      bool failed;
      std::stack<expected> context;
      std::stack<std::unique_ptr<json_node>> objects_being_built;
      std::stack<std::string> keys;

    public:
      builder_callback() : failed(false), context(), objects_being_built() {
      }

      void json_start() override {
        context.push(expected::value);
      }
      void json_end() override {
      }

      void json_string(const std::string& value) override {
        if (expects(expected::value)) {
          context.pop();
          attach(std::make_unique<json_string_node>(value));
        } else if (expects(expected::key)) {
          context.pop();
          context.push(expected::colon);
          keys.push(value);
        } else {
          fail("[string:" + value + "]");
        }
      }

      void json_number(double value) override {
        if (expects(expected::value)) {
          context.pop();
          attach(std::make_unique<json_numeric_node>(value));
        } else {
          fail("[double:" + to_string(value) + "]");
        }
      }

      void json_boolean(bool value) override {
        if (expects(expected::value)) {
          context.pop();
          attach(std::make_unique<json_boolean_node>(value));
        } else {
          fail("[boolean:" + to_string(value ? "true" : "false") + "]");
        }
      }

      void json_null() override {
        if (expects(expected::value)) {
          context.pop();
          attach(std::make_unique<json_null_node>());
        } else {
          fail("[null]");
        }
      }

      void json_comma() override {
        if (expects(expected::comma)) {
          context.pop();
          if (in_array()) {
            context.push(expected::value);
          } else if (in_object()) {
            context.push(expected::key);
          }
        } else {
          fail("[,(comma)]");
        }
      }

      void json_colon() override {
        if (expects(expected::colon)) {
          context.pop();
          context.push(expected::comma); // For next value.
          context.push(expected::value);
        } else {
          fail("[:(colon)]");
        }
      }

      void json_array_starts() override {
        context.push(expected::value);
        objects_being_built.push(std::make_unique<json_array_node>());
      }

      void json_array_ends() override {
        if (expects(expected::comma)) {
          context.pop(); // Pop the comma 
          context.pop(); // Pop the value
          auto value = std::move(objects_being_built.top()); // Move into local variable
          objects_being_built.pop();
          attach(std::move(value));
        } else {
          fail("[(value)]");
        }
      }

      void json_object_starts() override {
        context.push(expected::key);
        objects_being_built.push(std::make_unique<json_object_node>());
      }

      void json_object_ends() override {
        if (expects(expected::comma)) {
          context.pop(); // Pop the comma 
          context.pop(); // Pop the value
          auto value = std::move(objects_being_built.top()); // Move into local variable
          objects_being_built.pop();
          attach(std::move(value));
        } else {
          fail("[(key)]");
        }
      }

      void json_error(const std::string& error) override {
        throw json::json_error("Encountered an error during parse: " + error);
      }

      virtual bool need_more_json() override { return !failed && !expects(expected::none); }

      std::unique_ptr<json_node> result() {
        if (!need_more_json() && objects_being_built.size() == 1) {
          return std::move(objects_being_built.top());
        }
        throw json::json_error("Parsing process is not finished: [built=" + to_string(objects_being_built.size()) + "][need_more=" + to_string(need_more_json() ? "true" : "false") + "]");
      }

    private:
      bool expects(const expected& val) const {
        if (context.empty()) {
          return val == expected::none;
        }

        return (context.top() & val) != expected::none;
      }

      bool in_array() const {
        auto& current_object = objects_being_built.top();
        return current_object && current_object->type() == json_value_type::array;
      }

      bool in_object() const {
        auto& current_object = objects_being_built.top();
        return current_object && current_object->type() == json_value_type::object;
      }

      void attach(std::unique_ptr<json_node> value) {
        if (objects_being_built.empty()) {
          objects_being_built.push(std::move(value));
          return;
        }

        auto& current_object = objects_being_built.top();
        switch(current_object->type()) {
        case json_value_type::object:
          assert(!keys.empty());
          (*current_object)[keys.top()].reset(value.release());
          keys.pop();
          break;
        case json_value_type::array:
          static_cast<json::json_array_node*>(current_object.get())->push(std::move(value));
          context.push(expected::comma);
          break;
        default:
          objects_being_built.push(std::move(value));
        }
      }

      void fail(const std::string& unexpected) {
        failed = true;
        std::cerr << "Expected to see " << expected_values() << ", but got " << unexpected << "\n";
      }

      std::string expected_values() const {
        std::stringstream ss;
        if (expects(expected::value)) {
          ss << "[string, null, true, false, number]";
        }

        if (expects(expected::comma)) {
          ss << "[,(comma)]";
        }

        if (expects(expected::colon)) {
          ss << "[:(colon)]";
        }

        if (expects(expected::key)) {
          ss << "[:(colon)]";
        }
        
        return ss.str();
      }
    };

    std::unique_ptr<json_node> parse(const std::string& source) {
      builder_callback callback;
      run_tokenizer(source, callback);
      return callback.result();
    }
  }
}
