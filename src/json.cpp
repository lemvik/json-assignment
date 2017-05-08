#include "json.h"

#include <sstream>
#include <cassert>

namespace json {
  template<typename T>
  std::string to_string(const T& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  }

  std::ostream& operator<<(std::ostream& os, const value_type& val) {
    switch (val) {
    case value_type::null:    os << "null";    break;
    case value_type::number:  os << "number";  break;
    case value_type::boolean: os << "boolean"; break;
    case value_type::string:  os << "string";  break;
    case value_type::object:  os << "object";  break;
    case value_type::array:   os << "array";   break;
    }
    return os;
  }

  // Base class - defines most of methods
  struct value::value_impl {
    value_type type;

    value_impl(value_type t) : type(t) {}

    virtual ~value_impl() {}
    // Comparison operators
    virtual bool operator==(const value_impl& other) const {
      return type == other.type;
    }
    virtual bool operator!=(const value_impl& other) const {
      return type != other.type;
    }

    // Type checking
    bool is_null() const {
      return type == value_type::null;
    }
    bool is_string() const {
      return type == value_type::string;
    }
    bool is_number() const {
      return type == value_type::number;
    }
    bool is_boolean() const {
      return type == value_type::boolean;
    }
    bool is_object() const {
      return type == value_type::object;
    }
    bool is_array() const {
      return type == value_type::array;
    }
    value_type get_type() const {
      return type;
    }

    // Retrieving values
    virtual std::string as_string() const {
      throw json_error("Cannot return value of [type=" + to_string(type) + "] as string.");
    }
    virtual double as_number() const {
      throw json_error("Cannot return value of [type=" + to_string(type) + "] as double.");
    }
    virtual bool as_boolean() const {
      throw json_error("Cannot return value of [type=" + to_string(type) + "] as boolean.");
    }

    // Object-related stuff
    virtual bool has(const std::string&) const {
      throw json_error("Cannot query value of [type=" + to_string(type) + "] as object.");
    }
    virtual value& operator[](const std::string&) {
      throw json_error("Cannot query value of [type=" + to_string(type) + "] as object.");
    }
    virtual void remove(const std::string&) {
      throw json_error("Cannot remove value of [type=" + to_string(type) + "] since it's not an object.");
    }

    virtual value::object_iterator begin() const {
      throw json_error("Cannot iterate over value of [type=" + to_string(type) + "] as object.");
    }
    virtual value::object_iterator end() const {
      throw json_error("Cannot iterate over value of [type=" + to_string(type) + "] as object.");
    }
    virtual value::const_object_iterator cbegin() const {
      throw json_error("Cannot iterate over value of [type=" + to_string(type) + "] as object.");
    }
    virtual value::const_object_iterator cend() const {
      throw json_error("Cannot iterate over value of [type=" + to_string(type) + "] as object.");
    }

    // Array-related stuff
    virtual value& operator[](size_t) {
      throw json_error("Cannot query value of [type=" + to_string(type) + "] as array.");
    }
    virtual size_t push(value) {
      throw json_error("Cannot push to value of [type=" + to_string(type) + "] as array.");
    }
  };

  struct null_value : public value::value_impl {
    null_value() : value::value_impl(value_type::null) {}
  };

  class numeric_value : public value::value_impl {
    double val;
  public:
    numeric_value() : value::value_impl(value_type::number), val(0) {}
    numeric_value(double _val) : value::value_impl(value_type::number), val(_val) {}
    double as_number() const override {
      return val;
    }
  };

  class boolean_value : public value::value_impl {
    bool val;
  public:
    boolean_value() : value::value_impl(value_type::boolean), val(false) {}
    boolean_value(bool _val) : value::value_impl(value_type::boolean), val(_val) {}
    bool as_boolean() const override {
      return val;
    }
  };

  class string_value : public value::value_impl {
    std::string val;
  public:
    string_value() : value::value_impl(value_type::string), val("") {}
    string_value(const std::string& str) : value::value_impl(value_type::string), val(str) {}
    std::string as_string() const override {
      return val;
    }
  };

  class object_value : public value::value_impl {
    std::unordered_map<std::string, std::unique_ptr<value>> children;
  public:
    object_value() : value::value_impl(value_type::object), children() {}
    object_value(const object_value& other) : value::value_impl(value_type::object) {
      for (auto& el : other.children) {
        children.insert(std::make_pair(el.first, std::make_unique<value>(*el.second)));
      }
    }

    bool has(const std::string& key) const override {
      return children.find(key) != children.end();
    }

    value& operator[](const std::string& key) override {
      auto element_pair = children.emplace(key, std::make_unique<value>());
      return *(element_pair.first)->second;
    }

    void remove(const std::string& key) override {
      children.erase(key);
    }
  };

  class array_value : public value::value_impl {
    std::vector<std::unique_ptr<value>> values;
  public:
    array_value() : value::value_impl(value_type::array), values() {}
    array_value(const array_value& other) : value::value_impl(value_type::array), values() {
      for (auto& el : other.values) {
        values.push_back(std::make_unique<value>(*el));
      }
    }

    virtual value& operator[](size_t index) {
      if (index < values.size()) {
        return *(values[index]); // Since the returned value is fixed in memory (pinned by std::unique_ptr) only std::unique_ptr
                                 // is moved during resize, thus this value should be valid even if `values` vector resizes (as long as it contains
                                 // the appropriate pointer.
      }
      throw json_error("Given [index=" + to_string(index) + "] is out of bounds for the JSON array of [size=" + to_string(values.size()) + "]");
    }

    virtual size_t push(value val) {
      values.push_back(std::make_unique<value>(val));
      return values.size() - 1;
    }
  };

  std::unique_ptr<value::value_impl> static_dispatch_copy(const value::value_impl& source) {
    switch (source.type) {
    case value_type::null: return std::make_unique<null_value>(static_cast<const null_value&>(source));
    case value_type::boolean: return std::make_unique<boolean_value>(static_cast<const boolean_value&>(source));
    case value_type::number: return std::make_unique<numeric_value>(static_cast<const numeric_value&>(source));
    case value_type::string: return std::make_unique<string_value>(static_cast<const string_value&>(source));
    case value_type::object: return std::make_unique<object_value>(static_cast<const object_value&>(source));
    case value_type::array: return std::make_unique<array_value>(static_cast<const array_value&>(source));
    default:
      throw json_error("Unknown json type encountered: " + to_string(source.type) + " during copy construction.");
    }
  }

  value::~value() {}

  value::value(const value& other) : payload(static_dispatch_copy(*other.payload)) {
    // static_dispatch_copy here only to avoid doing reset() on payload.
  }

  value::value(value&& other) {
    using std::swap; // Actual design if from here http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom and Effective Modern C++ by Mayers
    swap(*this, other);
  }

  value& value::operator=(const value& other) {
    using std::swap;
    value new_other(other); // TODO: come up with more efficient solution.
    swap(*this, new_other); // Here we swap with the copy that gets thrown away anyways.
    return *this;
  }

  value& value::operator=(value&& other) {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  value::value() : payload(std::make_unique<null_value>()) {}
  value::value(const std::string& str) : payload(std::make_unique<string_value>(str)) {}
  value::value(const char* str) : payload(std::make_unique<string_value>(str)) {}
  value::value(double val) : payload(std::make_unique<numeric_value>(val)) {}
  value::value(bool val) : payload(std::make_unique<boolean_value>(val)) {}
  value::value(value_type t) : payload() {
    switch(t) {
    case value_type::null:    payload = std::make_unique<null_value>(); break;
    case value_type::boolean: payload = std::make_unique<boolean_value>(); break;
    case value_type::number:  payload = std::make_unique<numeric_value>(); break;
    case value_type::string:  payload = std::make_unique<string_value>(); break;
    case value_type::object:  payload = std::make_unique<object_value>(); break;
    case value_type::array:   payload = std::make_unique<array_value>(); break;
    default:
      throw json_error("Unknown [value_type=" + to_string(t) + "] encountered during construction.");
    }
  }

  value& value::operator=(const char* str) {        // Overload of char* assignment
    value nval(str);
    swap(*this, nval);
    return *this;
  }

  value& value::operator=(bool a) {               // Overload of bool assignment
    value nval{a};
    swap(*this, nval);
    return *this;
  }

  value& value::operator=(double d) {             // Overload of number assignment
    value nval{d};
    swap(*this, nval);
    return *this;
  }

  value& value::operator=(std::nullptr_t) {    // Overload of null assignment 
    value nval;
    swap(*this, nval);
    return *this;
  }

  bool value::operator==(const value& other) const {
    return *payload == *other.payload;
  }

  bool value::operator!=(const value& other) const {
    return *payload != *other.payload;
  }

  bool value::is_null() const { return payload->is_null(); }
  bool value::is_string() const { return payload->is_string(); }
  bool value::is_number() const { return payload->is_number(); }
  bool value::is_boolean() const { return payload->is_boolean(); }
  bool value::is_object() const { return payload->is_object(); }
  bool value::is_array() const { return payload->is_array(); }
  value_type value::get_type() const { return payload->get_type(); }

  std::string value::as_string() const { return payload->as_string(); }
  double      value::as_number() const { return payload->as_number(); }
  bool        value::as_boolean() const { return payload->as_boolean(); }

  bool value::has(const std::string& key) const { return payload->has(key); }
  value& value::operator[](const std::string& key) { return (*payload)[key]; }
  void value::remove(const std::string& key) { return payload->remove(key); }

  value::object_iterator value::begin() const { return payload->begin(); }
  value::object_iterator value::end() const { return payload->end(); }
  value::const_object_iterator value::cbegin() const { return payload->cbegin(); }
  value::const_object_iterator value::cend() const { return payload->cend(); }

  value& value::operator[](size_t index) { return (*payload)[index]; }
  size_t value::push(value other) { return payload->push(other); }

  void swap(value& lhs, value& rhs) {
    using std::swap;
    swap(lhs.payload, rhs.payload);
  }
}
