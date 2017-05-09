#ifndef _JSON_H_
#define _JSON_H_

#include <unordered_map>
#include <vector>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <memory>

namespace json {

  // Possible types of JSON values.
  enum class value_type { null, number, boolean, string, object, array };
  std::ostream& operator<<(std::ostream& os, const value_type& val); 

  // Error to signal if anything goes awry.
  class json_error : public std::runtime_error {
  public:
    json_error(const std::string& reason) : std::runtime_error(reason) {}
  };

  struct value {
    // Constructor and assignment stuff
    ~value();
    value(const value&);
    value(value&&);
    value& operator=(const value&);
    value& operator=(value&&);

    // Custom constructors from primitives
    value();                   // Constructs null value
    value(const std::string&); // Constructs string value
    value(const char*);        // Constructs string value (this one is needed thanks to `wonderful` preference of bool-constructor to std::string one).
    explicit value(bool);      // Constructs boolean node
    value(double);             // Constructs numeric value
    value(value_type type);    // Constructs any type of value wanted.

    // Following methods are only needed because I was tired fighting
    // C++ moronic overload resolution rules:
    // 1. For some reason `val = 0.0` resolves into a call to value(bool) constructor...
    // 2. Same for `val = "str"`...
    // 3. Presence of operator=(bool) breaks `val = 0.0` if it's not also overloaded...
    value& operator=(const std::string&);
    value& operator=(const char*);        // Overload of char* assignment
    value& operator=(bool);               // Overload of bool assignment
    value& operator=(double);             // Overload of number assignment
    value& operator=(std::nullptr_t);     // Overload of null assignment 

    // Comparison operators
    bool operator==(const value&) const;
    bool operator!=(const value&) const;

    // Type checking
    bool is_null() const;
    bool is_string() const;
    bool is_number() const;
    bool is_boolean() const;
    bool is_object() const;
    bool is_array() const;
    value_type get_type() const;

    // Retrieving values
    std::string as_string() const;
    double      as_number() const;
    bool        as_boolean() const;

    // Object-related stuff
    bool has(const std::string&) const;
    value& operator[](const std::string&);
    void remove(const std::string&);

    // NOTE: references to pairs returned by this iterator are valid ONLY until
    // next ++ operator. It can be done in other way, but for simplicity it's so for now
    // Note that references returned in the `second` of the pair are valid for as long
    // as container contains the referenced value
    using object_entry = std::pair<const std::string&, value&>;
    struct object_iterator : public std::iterator<std::forward_iterator_tag, object_entry> {
      struct object_iterator_impl;
      object_iterator(const object_iterator& other);
      object_iterator(const value& source);
      ~object_iterator();
      object_iterator& to_end();
      object_iterator& operator++();
      object_iterator operator++(int) {object_iterator res = *this; ++(*this); return res;}
      bool operator==(object_iterator other) const;
      bool operator!=(object_iterator other) const;
      reference operator*() const;
    private:
      std::unique_ptr<object_iterator_impl> impl;
    };
    object_iterator begin() const;
    object_iterator end() const;

    struct array_iterator : public std::iterator<std::forward_iterator_tag, value> {
      struct array_iterator_impl;
      array_iterator(const array_iterator& other);
      array_iterator(value& source);
      ~array_iterator();
      array_iterator& to_end();
      array_iterator& operator++();
      array_iterator operator++(int) {array_iterator res = *this; ++(*this); return res;}
      bool operator==(array_iterator other) const;
      bool operator!=(array_iterator other) const;
      reference operator*() const;
    private:
      std::unique_ptr<array_iterator_impl> impl;
    };
    array_iterator abegin();
    array_iterator aend();

    // Array-related stuff
    value& operator[](size_t);
    size_t push(value);

    friend void swap(value& lhs, value& rhs);
    struct value_impl;
  private:
    std::unique_ptr<value_impl> payload;
  };
}

#endif
