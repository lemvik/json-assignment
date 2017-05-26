#include "json.h"
#include "utils.h"

#include <cassert>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace json {

  // This code is "borrowed" from here: http://stackoverflow.com/a/33799784
  // This stability not really required in this assignment, however, it's `proven to work` as
  // in `I trust everything I can find on the Web`
  std::string escape(const std::string& str) {
    std::ostringstream o;
    o << '"';
    for (const auto& c : str) {
        switch (c) {
        case '"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if ('\x00' <= c && c <= '\x1f') {
                o << "\\u"
                  << std::hex << std::setw(4) << std::setfill('0') << (int)c;
            } else {
                o << c;
            }
        }
    }
    o << '"';
    return o.str();
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

  // Releases resources held by this instance.
  // Does not throw.
  value::~value() {
    release();
  }

  // This function is noexcept because ~string() doesn't throw by standard
  // and ~vector() and ~unordered_map() are _quite likely_ do not throw if destructors
  // for contained items do not throw. Since items here are string and value, whose
  // destructors do not throw, it's arguably safe to assume this method doesn't throw.
  void value::release() noexcept {
    using std::string;
    using std::unordered_map;
    using std::vector;

    switch (type) {
    case value_type::null: 
    case value_type::number: 
    case value_type::boolean: 
      break;
    case value_type::string:
      this->string.~string();
      break;
    case value_type::object:
      this->object.~unordered_map();
      break;
    case value_type::array:
      this->array.~vector();
      break;
    }
  }

  // Assertion-like function that checks if given value is of appropriate type for operation.
  // Throws exception with explanation if it is not so.
  void should_be(const value& val, value_type t) {
    if (val.get_type() != t) {
      throw json_error("Value of [type=" + utils::to_string(val.get_type()) + "] is treated as value of [type=" + utils::to_string(t) + "]");
    }
  }

  // Copies contents from other value.
  void value::from(const value& other) {
    assert(type == other.type); // This method is assumed to be invoked after type is properly set.
                                // Although not super-good decision, it allows setting type in initalizer expression
                                // of copy-constructor.
    switch (other.type) {
    case value_type::null:    break; // Leave union unitialized.
    case value_type::number:  this->number  = other.number;  break; 
    case value_type::boolean: this->boolean = other.boolean; break; 
    case value_type::string:  this->string  = other.string;  break;
    case value_type::object:
      for (auto& el : other.object) {
        this->object.insert(std::make_pair(el.first, std::make_unique<value>(*el.second)));
      }
      break;
    case value_type::array:
      for (auto& el : other.array) {
        this->array.push_back(std::make_unique<value>(*el));
      }
      break;
    }
  }

  value::value(const value& other) : value(other.type) {
    from(other);
  }

  value::value(value&& other) : value(other.type) {
    switch (other.type) {
    case value_type::null:    this->number  = 0;             break;
    case value_type::number:  this->number  = other.number;  break; 
    case value_type::boolean: this->boolean = other.boolean; break; 
    case value_type::string:  new (&string) std::string(std::move(other.string)); break;
    case value_type::object:  new (&object) std::unordered_map<std::string, std::unique_ptr<value>>(std::move(other.object)); break;
    case value_type::array:   new (&array) std::vector<std::unique_ptr<value>>(std::move(other.array));  break;
    }
  }

  value& value::operator=(const value& other) {
    if (this == &other) {
      return *this;
    }
    release();
    type = other.type;
    from(other);
    return *this;
  }

  value& value::operator=(value&& other) {
    release();
    type = other.type;
    switch (other.type) {
    case value_type::null:    break;
    case value_type::number:  this->number  = other.number;  break; 
    case value_type::boolean: this->boolean = other.boolean; break; 
    case value_type::string:  new (&string) std::string(std::move(other.string)); break;
    case value_type::object:  new (&object) std::unordered_map<std::string, std::unique_ptr<value>>(std::move(other.object)); break;
    case value_type::array:   new (&array) std::vector<std::unique_ptr<value>>(std::move(other.array));  break;
    }
    return *this;
  }

  value::value() : type(value_type::null) {}
  value::value(std::nullptr_t) : type(value_type::null) {}
  value::value(const std::string& str) : type(value_type::string), string(str) {}
  value::value(const char* str) : type(value_type::string), string(str) {}
  value::value(double val) : type(value_type::number), number(val) {}
  value::value(bool val) : type(value_type::boolean), boolean(val) {}
  value::value(std::initializer_list<std::pair<std::string, value>> pairs) : type(value_type::object), object() {
    for (auto el : pairs) {
      object[el.first] = std::make_unique<value>(el.second);
    }
  }

  value::value(value_type t) : type(t) {
    switch(t) {
    case value_type::null:    break;
    case value_type::boolean: boolean = false; break;
    case value_type::number:  number = 0;      break; 
    case value_type::string:  new (&string) std::string(); break;
    case value_type::object:  new (&object) std::unordered_map<std::string, std::unique_ptr<value>>(); break;
    case value_type::array:   new (&array) std::vector<std::unique_ptr<value>>(); break;
    default:
      throw json_error("Unknown [value_type=" + utils::to_string(t) + "] encountered during construction.");
    }
  }

  value& value::operator=(const char* str) {
    // TODO: think about exception safety
    release();
    type = value_type::string;
    new (&string) std::string(str);
    return *this;
  }

  value& value::operator=(bool a) {
    release();
    type = value_type::boolean;
    boolean = a;
    return *this;
  }

  value& value::operator=(double d) {
    release();
    type = value_type::number;
    number = d;
    return *this;
  }

  value& value::operator=(std::nullptr_t) {
    release();
    type = value_type::null;
    number = 0;
    return *this;
  }

  bool value::operator==(const value& other) const {
    if (type != other.type) return false;

    switch (type) {
    case value_type::null:    return true;
    case value_type::boolean: return boolean == other.boolean;
    case value_type::number:  return number == other.number;
    case value_type::string:  return string == other.string;
    case value_type::object:  return object == other.object;
    case value_type::array:   return array == other.array;
    default:
      // We shouldn't end up here, but we might, since enum class can be
      // operated upon via static_cast<int> + bitwise operations + cast back and C++ standard
      // mandates that such operations are valid.
      assert(false);
      return false;
    }
  }

  bool value::operator!=(const value& other) const {
    return !((*this) == other);
  }

  bool value::operator==(std::nullptr_t) const {
    return type == value_type::null;
  }
  bool value::operator!=(std::nullptr_t) const {
    return type != value_type::null;
  }

  bool value::is_null() const { return type == value_type::null; }
  bool value::is_string() const { return type == value_type::string; }
  bool value::is_number() const { return type == value_type::number; }
  bool value::is_boolean() const { return type == value_type::boolean; }
  bool value::is_object() const { return type == value_type::object; }
  bool value::is_array() const { return type == value_type::array; }
  value_type value::get_type() const { return type; }

  std::string value::as_string() const { should_be(*this, value_type::string); return string; }
  double      value::as_number() const { should_be(*this, value_type::number); return number; }
  bool        value::as_boolean() const { should_be(*this, value_type::boolean); return boolean; }

  std::string value::serialize() const {
    switch (type) {
    case value_type::null:    return "null";
    case value_type::boolean: return boolean ? "true" : "false";
    case value_type::number:  return utils::to_string(number);
    case value_type::string:  return escape(string);
    case value_type::object: {
      std::stringstream ss;
      std::string separator = "";
      ss << "{";
      for (const auto& el: object) {
        ss << separator;
        ss << escape(el.first) << ":" << el.second->serialize();
        separator = ",";
      }
      ss << "}";
      return ss.str();
    }
    case value_type::array: {
      std::stringstream ss;
      std::string separator = "";
      ss << "[";
      for (const auto& el: array) {
        ss << separator;
        ss << el->serialize();
        separator = ",";
      }
      ss << "]";
      return ss.str();
    }
    default:
      // See comment at operator==
      assert(false);
      return std::string(); // Unreachable actually, but no standard way to avoid warning AFAIK
    }
  }

  bool value::has(const std::string& key) const { should_be(*this, value_type::object); return object.find(key) != object.end(); }
  value& value::operator[](const std::string& key) {
    should_be(*this, value_type::object);
    auto element = object.emplace(key, std::make_unique<value>());
    return *(element.first->second);
  }
  void value::remove(const std::string& key) { should_be(*this, value_type::object); object.erase(key); }

  value& value::operator[](size_t index) {
    should_be(*this, value_type::array);
    if (index >= array.size()) {
      throw std::out_of_range("Given [index=" + utils::to_string(index) + "] is out of bounds for the JSON array of [size=" + utils::to_string(array.size()) + "]");
    }
    return *(array[index]);
  }
  size_t value::push(value other) { should_be(*this, value_type::array); array.push_back(std::make_unique<value>(other)); return array.size() - 1; }
  size_t value::remove(size_t index) {
    should_be(*this, value_type::array);
    if (index >= array.size()) {
      throw std::out_of_range("Given [index=" + utils::to_string(index) + "] is out of bounds for the JSON array of [size=" + utils::to_string(array.size()) + "]");
    }
    array.erase(array.begin() + index);
    return array.size();
  }

  size_t value::size() const {
    switch (type) {
    case value_type::array:
      return array.size();
    case value_type::object:
      return object.size();
    default:
      throw json_error("Can only query size of object and array nodes, this node type is [type=" + utils::to_string(type) + "]");
    }
  }

  bool value::empty() const {
    switch (type) {
    case value_type::array:
      return array.empty();
    case value_type::object:
      return object.empty();
    default:
      throw json_error("Can only query emptiness of object and array nodes, this node type is [type=" + utils::to_string(type) + "]");
    }
  }

  void swap(value& lhs, value& rhs) {
    using std::swap;
    if (lhs.type == rhs.type) {
      switch (lhs.type) {
      case value_type::null:    break;
      case value_type::boolean: swap(lhs.boolean, rhs.boolean);
      case value_type::number:  swap(lhs.number, rhs.number);
      case value_type::string:  swap(lhs.string, rhs.string);
      case value_type::object:  swap(lhs.object, rhs.object);
      case value_type::array:   swap(lhs.array, rhs.array);
      }
      return;
    }

    std::swap(lhs, rhs); // This swap will use move construction and two move assignments
  }

  value::object_iterator::object_iterator(const value::object_iterator& other) : source(other.source) {
    if (other.value_reference) {
      value_reference = std::make_unique<object_entry>(*other.value_reference);
    }
  }
  value::object_iterator::object_iterator(const std::unordered_map<std::string, std::unique_ptr<value>>::const_iterator& src) : source(src), value_reference(nullptr) {
    source = src; 
  }
  value::object_iterator::~object_iterator() = default;
  value::object_iterator& value::object_iterator::operator++() {
    ++source;
    return *this;
  }
  bool value::object_iterator::operator==(value::object_iterator other) const { return source == other.source; }
  bool value::object_iterator::operator!=(value::object_iterator other) const { return source != other.source; }
  value::object_iterator::reference value::object_iterator::operator*() const { 
    value_reference = std::make_unique<value::object_entry>(source->first, *(source->second));
    return *value_reference;
  }

  value::array_iterator::array_iterator(const array_iterator& other) : source(other.source) {}
  value::array_iterator::array_iterator(const std::vector<std::unique_ptr<value>>::const_iterator& src) : source(src) {}
  value::array_iterator::~array_iterator() = default; 
  value::array_iterator& value::array_iterator::operator++() {
    ++source;
    return *this;
  }
  bool value::array_iterator::operator==(value::array_iterator other) const { return source == other.source; }
  bool value::array_iterator::operator!=(value::array_iterator other) const { return source != other.source; }
  value::array_iterator::reference value::array_iterator::operator*() const {
    return *(*source);
  }

  array_value value::as_array() {
    should_be(*this, value_type::array);
    return array_value(*this);
  }
  object_value value::as_object() {
    should_be(*this, value_type::object);
    return object_value(*this);
  }
  
  std::ostream& operator<<(std::ostream& os, const value& val) {
    return os << val.serialize();
  }

  array_value::array_value(value& _value) : wrapped_value(_value) {}
  array_value::array_value(array_value&& other) : wrapped_value(other.wrapped_value) {} // Not `move` per se
  array_value& array_value::operator=(array_value&& other) {
    wrapped_value = other.wrapped_value;
    return *this;
  }

  value& array_value::operator[](size_t index) {
    // Since we'd like to avoid runtime checks:
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    if (index >= array.size()) {
      throw std::out_of_range("Given [index=" + utils::to_string(index) + "] is out of bounds for the JSON array of [size=" + utils::to_string(array.size()) + "]");
    }
    return *(array[index]);
  }
  size_t array_value::push(value other) {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    array.push_back(std::make_unique<value>(other));
    return array.size() - 1;
  }
  size_t array_value::remove(size_t index) {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    if (index >= array.size()) {
      throw std::out_of_range("Given [index=" + utils::to_string(index) + "] is out of bounds for the JSON array of [size=" + utils::to_string(array.size()) + "]");
    }
    array.erase(array.begin() + index);
    return array.size();
  }
  size_t array_value::size() const {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    return array.size();
  }
  bool   array_value::empty() const {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    return array.empty();
  }
  value::array_iterator array_value::begin() {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    return value::array_iterator(array.begin());
  }
  value::array_iterator array_value::end() {
    assert(wrapped_value.type == value_type::array);
    auto& array = wrapped_value.array;
    return value::array_iterator(array.end());
  }

  object_value::object_value(value& _value) : wrapped_value(_value) {}
  object_value::object_value(object_value&& other) : wrapped_value(other.wrapped_value) {} // Not `move` per se
  object_value& object_value::operator=(object_value&& other) {
    wrapped_value = other.wrapped_value;
    return *this;
  }

  bool   object_value::has(const std::string& key) const {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    return object.find(key) != object.end();
  }
  value& object_value::operator[](const std::string& key) {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    auto element = object.emplace(key, std::make_unique<value>());
    return *(element.first->second);
  }
  void   object_value::remove(const std::string& key) {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    object.erase(key);
  }
  size_t object_value::size() const {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    return object.size();
  }
  bool   object_value::empty() const {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    return object.empty();
  }
  value::object_iterator object_value::begin() {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    return value::object_iterator(object.begin());
  }
  value::object_iterator object_value::end() {
    assert(wrapped_value.type == value_type::object);
    auto& object = wrapped_value.object;
    return value::object_iterator(object.end());
  }
}
