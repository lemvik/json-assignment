#include "json.h"

#include <sstream>
#include <iomanip>
#include <cassert>

namespace json {
  template<typename T>
  std::string to_string(const T& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  }

  // This code is "borrowed" from here: http://stackoverflow.com/a/33799784
  std::string escape(std::string str) {
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

    virtual std::string serialize() const {
      throw json_error("Cannot serialize base json node.");
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

    // Array-related stuff
    virtual value& operator[](size_t) {
      throw json_error("Cannot query value of [type=" + to_string(type) + "] as array.");
    }
    virtual size_t push(value) {
      throw json_error("Cannot push to value of [type=" + to_string(type) + "] as array.");
    }

    virtual size_t size() const {
      throw json_error("Cannot query size of value of [type=" + to_string(type) + "].");
    }

    virtual bool empty() const {
      throw json_error("Cannot query emptiness of value of [type=" + to_string(type) + "].");
    }
  };

  struct null_value : public value::value_impl {
    null_value() : value::value_impl(value_type::null) {}
    std::string serialize() const override {
      return "null";
    }
  };

  class numeric_value : public value::value_impl {
    double val;
  public:
    numeric_value() : value::value_impl(value_type::number), val(0) {}
    numeric_value(double _val) : value::value_impl(value_type::number), val(_val) {}
    double as_number() const override {
      return val;
    }
    std::string serialize() const override {
      return to_string(val);
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
    std::string serialize() const override {
      return val ? "true" : "false";
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
    std::string serialize() const override {
      return escape(val);
    }
  };

  class object_value : public value::value_impl {
    std::unordered_map<std::string, std::unique_ptr<value>> children;
    friend value::object_iterator::object_iterator(const value&);
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

    size_t size() const override {
      return children.size();
    }

    bool empty() const override {
      return children.empty();
    }
    
    std::string serialize() const override {
      std::stringstream ss;
      auto sep = "";

      ss << "{";

      for (auto& el : children) {
        ss << sep << '"' << el.first << '"' << ":" << el.second->serialize();
        sep = ",";
      }
      
      ss << "}";

      return ss.str();
    }
  };

  class array_value : public value::value_impl {
    std::vector<std::unique_ptr<value>> values;
    friend value::array_iterator::array_iterator(value&);
  public:
    array_value() : value::value_impl(value_type::array), values() {}
    array_value(const array_value& other) : value::value_impl(value_type::array), values() {
      for (auto& el : other.values) {
        values.push_back(std::make_unique<value>(*el));
      }
    }

    value& operator[](size_t index) override {
      if (index < values.size()) {
        return *(values[index]); // Since the returned value is fixed in memory (pinned by std::unique_ptr) only std::unique_ptr
                                 // is moved during resize, thus this value should be valid even if `values` vector resizes (as long as it contains
                                 // the appropriate pointer.
      }
      throw json_error("Given [index=" + to_string(index) + "] is out of bounds for the JSON array of [size=" + to_string(values.size()) + "]");
    }

    size_t push(value val) override {
      values.push_back(std::make_unique<value>(val));
      return values.size() - 1;
    }

    size_t size() const override {
      return values.size();
    }

    bool empty() const override {
      return values.empty();
    }

    std::string serialize() const override {
      std::stringstream ss;
      auto sep = "";

      ss << "[";

      for (auto& el : values) {
        ss << sep << el->serialize();
        sep = ",";
      }
      
      ss << "]";

      return ss.str();
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
  value::value(std::nullptr_t) : payload(std::make_unique<null_value>()) {}
  value::value(const std::string& str) : payload(std::make_unique<string_value>(str)) {}
  value::value(const char* str) : payload(std::make_unique<string_value>(str)) {}
  value::value(double val) : payload(std::make_unique<numeric_value>(val)) {}
  value::value(bool val) : payload(std::make_unique<boolean_value>(val)) {}
  // value::value(std::initializer_list<value> vals) : payload(std::make_unique<array_value>()) {
  //   for (auto el : vals) {
  //     payload->push(el);
  //   }
  // }
  value::value(std::initializer_list<std::pair<std::string, value>> pairs) : payload(std::make_unique<object_value>()) {
    for (auto el : pairs) {
      (*payload)[el.first] = el.second;
    }
  }

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
    value nval(a);
    swap(*this, nval);
    return *this;
  }

  value& value::operator=(double d) {             // Overload of number assignment
    value nval(d);
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

  bool value::operator==(std::nullptr_t) const {
    return get_type() == value_type::null;
  }
  bool value::operator!=(std::nullptr_t) const {
    return get_type() != value_type::null;
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

  std::string value::serialize() const { return payload->serialize(); }

  bool value::has(const std::string& key) const { return payload->has(key); }
  value& value::operator[](const std::string& key) { return (*payload)[key]; }
  void value::remove(const std::string& key) { return payload->remove(key); }

  value::object_iterator value::begin() const {
    return value::object_iterator(*this);
  }
  value::object_iterator value::end() const {
    return value::object_iterator(*this).to_end();
  }
  value::array_iterator value::abegin() {
    return value::array_iterator(*this);
  }
  value::array_iterator value::aend() {
    return value::array_iterator(*this).to_end();
  }

  value& value::operator[](size_t index) { return (*payload)[index]; }
  size_t value::push(value other) { return payload->push(other); }

  size_t value::size() const { return payload->size(); }
  bool value::empty() const { return payload->empty(); }

  void swap(value& lhs, value& rhs) {
    using std::swap;
    swap(lhs.payload, rhs.payload);
  }

  struct value::object_iterator::object_iterator_impl : public std::iterator<std::forward_iterator_tag, value::object_entry> {
    using source = std::unordered_map<std::string, std::unique_ptr<value>>;
    using source_iterator = source::const_iterator;

    const source& src;
    source_iterator iter;
    std::unique_ptr<value::object_entry> current_reference; // Should be std::optional in C++ 17

    object_iterator_impl(const source& _src) : src(_src), iter(src.begin()) { update_reference(); }
    object_iterator_impl(const source& _src, source_iterator _iter) : src(_src), iter(_iter) { update_reference(); }
    object_iterator_impl(const object_iterator_impl& other) : src(other.src), iter(other.iter) { update_reference(); }

    void update_reference() {
      if (iter != src.end()) {
        current_reference = std::make_unique<value::object_entry>(iter->first, *(iter->second));
      }
    }
  };

  value::object_iterator::object_iterator(const value::object_iterator& other) : impl(std::make_unique<object_iterator_impl>(*other.impl)) {  }
  value::object_iterator::object_iterator(const value& source) {
    if (source.get_type() != json::value_type::object) {
      throw json_error("Unable to use object_iterator to iterate oven non-object: [type=" + to_string(source.get_type()) + "]");
    }

    auto& map = static_cast<object_value&>(*source.payload).children;
    impl = std::make_unique<object_iterator_impl>(map);
  }
  value::object_iterator::~object_iterator() = default;
  value::object_iterator& value::object_iterator::operator++() {
    ++(impl->iter);
    impl->update_reference();
    return *this;
  }
  value::object_iterator& value::object_iterator::to_end() {
    auto last = impl->src.end();
    const auto& src = impl->src;
    impl = std::make_unique<object_iterator_impl>(src, last); 
    return *this;
  }
  bool value::object_iterator::operator==(value::object_iterator other) const { return impl->iter == other.impl->iter; }
  bool value::object_iterator::operator!=(value::object_iterator other) const { return impl->iter != other.impl->iter; }
  value::object_iterator::reference value::object_iterator::operator*() const { return *(impl->current_reference); }

  struct value::array_iterator::array_iterator_impl {
    using source = std::vector<std::unique_ptr<value>>;
    using iterator = std::vector<std::unique_ptr<value>>::iterator;
    
    source& src;
    iterator iter;

    array_iterator_impl(source& _src, iterator _iter) : src(_src), iter(_iter) {}
    array_iterator_impl(const array_iterator_impl& other) : src(other.src), iter(other.iter) {}
  };

  value::array_iterator::array_iterator(const array_iterator& other) : impl(std::make_unique<array_iterator_impl>(*other.impl)) {}
  value::array_iterator::array_iterator(value& source) {
    if (source.get_type() != json::value_type::array) {
      throw json_error("Unable to construct array iterator from value of [type=" + to_string(source.get_type()) + "]");
    }
    auto& vector = static_cast<array_value&>(*source.payload).values;
    impl = std::make_unique<array_iterator_impl>(vector, vector.begin());
  }
  value::array_iterator::~array_iterator() = default; 
  value::array_iterator& value::array_iterator::to_end() {
    auto& vec = impl->src;
    auto iter = vec.end();
    impl = std::make_unique<array_iterator_impl>(vec, iter);
    return *this;
  }
  value::array_iterator& value::array_iterator::operator++() { ++(impl->iter); return *this; }
  bool value::array_iterator::operator==(value::array_iterator other) const { return impl->iter == other.impl->iter; }
  bool value::array_iterator::operator!=(value::array_iterator other) const { return impl->iter != other.impl->iter; }
  value::array_iterator::reference value::array_iterator::operator*() const {
    return *(*(impl->iter)); // Since we want to return reference - two dereferences.
  }

  std::ostream& operator<<(std::ostream& os, const value& val) {
    return os << val.serialize();
  }
}
