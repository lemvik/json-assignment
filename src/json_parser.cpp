#include "json.h"
#include "json_parser.h"
#include "json_sa.h"
#include "utils.h"

#include <cassert>
#include <stack>
#include <sstream>
#include <iostream>

namespace json {
  namespace parser {

    // The parsing process is implemented via stateful token_callback. It maintains current parsing context and
    // attachment points (either an array or an object).

    // Value representing next expected token - none is not really needed
    enum class next_token : int { none, value, comma, colon, key };

    class builder_callback : public simple::token_callback {
      bool failed;
      value root;                             // By default we will have null value.
      std::stack<next_token> context;         // Stack of expected tokens - embodiment of the parsing state machine.
      std::stack<value*> objects_being_built; // Stack of non-owning pointers, for in-place building. The state of parse fully determined by context and object_being_built.
      std::stack<std::string> keys;

    public:
      builder_callback() : failed(false), root(), context(), objects_being_built(), keys() {}

      // At the start of parsing process we expect to see a single top level value.
      void json_start() override {
        context.push(next_token::value);
      }

      // Usage scenario assumes this callback won't be reused, so we do not cleanup anything.
      void json_end() override {}

      // The string we receive could be either key or a value.
      void json_string(const std::string& str) override {
        if (expects(next_token::value)) {
          attach(value(str));
          context.pop();
          value_read();
        } else if (expects(next_token::key)) {
          context.pop();
          context.push(next_token::colon);
          keys.push(str);
        } else {
          fail("[string:" + str + "]");
        }
      }

      // Number is always a value
      void json_number(double num) override {
        if (expects(next_token::value)) {
          attach(value(num));
          context.pop();
          value_read();
        } else {
          fail("[double:" + utils::to_string(num) + "]");
        }
      }

      // Boolean is always a value.
      void json_boolean(bool flag) override {
        if (expects(next_token::value)) {
          attach(value(flag));
          context.pop();
          value_read();
        } else {
          fail("[boolean:" + utils::to_string(flag ? "true" : "false") + "]");
        }
      }

      // Null is always a value.
      void json_null() override {
        if (expects(next_token::value)) {
          attach(value());
          context.pop();
          value_read();
        } else {
          fail("[null]");
        }
      }

      // Comma can separate entries in array or object, so we check if we are in_array or in_object
      void json_comma() override {
        if (expects(next_token::comma)) {
          context.pop();
          if (in_array()) {
            context.push(next_token::value);
          } else if (in_object()) {
            context.push(next_token::key);
          } else {
            assert(!"Encountered a comma, but not building an object or an array.");
          }
        } else {
          fail("[,(comma)]");
        }
      }

      // Colon can be encountered only as separator between object key and object value.
      void json_colon() override {
        if (expects(next_token::colon)) {
          assert(in_object());
          context.pop();
          context.push(next_token::value);
        } else {
          fail("[:(colon)]");
        }
      }

      // Array is always a value.
      void json_array_starts() override {
        if (expects(next_token::value)) {
          objects_being_built.push(attach(value_type::array));
          context.push(next_token::value);
        } else {
          fail("[(array)]");
        }
      }

      // Valid state for json_array_end:
      // 1. We are building array (in_array() == true)
      // 2. Either:
      //    a. Array being built is empty and we expect first value to appear
      //    b. We have already built some values and are currently expecting a comma
      void json_array_ends() override {
        if (!in_array()) {
          fail("[(array_end)]");
          return; // Not strictly necessary here as fail will throw.
        }

        if ((expects(next_token::value) && empty_attach_point()) || expects(next_token::comma)) {
          context.pop(); // Pop the value (expected first value of the array) or comma
          assert(expects(next_token::value));
          context.pop(); // Pop the value (the value for the array itself).
          objects_being_built.pop();
          value_read();
        } else {
          fail("[(array_end)]");
        }
      }

      // Object is always a value.
      void json_object_starts() override {
        if (expects(next_token::value)) {
          objects_being_built.push(attach(value_type::object));
          context.push(next_token::key);
        } else {
          fail("[(object)]");
        }
      }

      // Valid state for json_object_end:
      // 1. We are building object (in_object() == true)
      // 2. Either:
      //    a. Object being built is empty and we expect first key to appear
      //    b. We have already built some key:value pairs and are currently expecting a comma
      void json_object_ends() override {
        if (!in_object()) {
          fail("[(object_end)]");
          return; // Not strictly necessary here as fail will throw.
        }

        if ((expects(next_token::key) && empty_attach_point()) || expects(next_token::comma)) {
          context.pop(); // Pop the expected key/comma
          assert(expects(next_token::value));
          context.pop(); // Pop the value
          objects_being_built.pop();
          value_read();
        } else {
          fail("[(object_end)]");
        }
      }

      // All errors get thrown out as json_errors (to avoid exposing parser_error)
      void json_error(const std::string& error) override {
        failed = true;
        throw json::json_error("Encountered an error during parse: " + error);
      }

      // We assume that parsing should stop when we either failed, or do not expect anything else.
      virtual bool need_more_json() override { return !failed && !expects(next_token::none); }

      // If for some reason someone attempts to query restul when we haven't finished parsing
      // we throw out the error.
      value result() {
        if (!need_more_json()) {
          return root; 
        }
        throw json::json_error("Parsing process is not finished: [built=" + utils::to_string(objects_being_built.size()) + "][need_more=" + utils::to_string(need_more_json() ? "true" : "false") + "]");
      }

    private:

      // Check if we expect the given value type
      bool expects(const next_token& val) const {
        if (context.empty()) {
          return val == next_token::none;
        }

        return context.top() == val;
      }

      // We are in array if top level object_being_built is of type array.
      inline bool in_array() const {
        return in(value_type::array);
      }

      // We are in object if top level object_being_built is of type object.
      inline bool in_object() const {
        return in(value_type::object);
      }

      // Common code for in_object/in_array
      bool in(value_type t) const {
        if (objects_being_built.empty()) {
          return false;
        }
        auto current_object = objects_being_built.top();
        return current_object && current_object->get_type() == t;
      }

      // We have empty attachment point if its either an array or an object and has empty() == true
      bool empty_attach_point() const {
        assert(!objects_being_built.empty());
        auto current_object = objects_being_built.top();
        auto obj_type = current_object->get_type();
        bool is_container = (obj_type == value_type::array) || (obj_type == value_type::object);
        assert(is_container);
        return current_object->empty();
      }

      // Attaches freshly parsed value to one currently being built. If no objects
      // are being built - replace root.
      value* attach(value&& value) {
        assert(expects(next_token::value));
        if (objects_being_built.empty()) {
          root = std::move(value);
          return &root;
        }

        auto& current_object = objects_being_built.top();
        switch(current_object->get_type()) {
        case value_type::object: {
          const std::string& key = keys.top();
          auto* value_p = &((*current_object)[key] = std::move(value)); // Store pointer as per recommendation.
          keys.pop();
          return value_p;
        }
        case value_type::array: {
          size_t last_index = (*current_object).push(std::move(value));
          return &((*current_object)[last_index]);
        }
        default:
          // Not sure if this is good style, but no other way to print custom error message from stock assert.
          assert(false && ("Cannot attach to a [type=" + utils::to_string(current_object->get_type()) + "] value").c_str()); 
          return nullptr; // To avoid compiler warnings
        }
      }

      // Convenience function to push comma if need be.
      void value_read() {
        if (in_array() || in_object()) {
          context.push(next_token::comma);
        }
      }

      // Failure essentially means exeption thrown 
      void fail(const std::string& reason) {
        failed = true;
        std::stringstream ss;
        ss << "Unable to parse JSON: expected to see " << next_token_values() << ", but got " << reason << "\n";
        throw json::json_error(ss.str());
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
          ss << "[object_key]";
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
