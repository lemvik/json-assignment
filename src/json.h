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
    template<typename T>
    value(T) = delete;
    value();                                                     // Constructs default null value
    value(std::nullptr_t);                                       // Constructs null value explicitly
    value(const std::string&);                                   // Constructs string value
    value(const char*);                                          // Constructs string value (this one is needed thanks to `wonderful` preference of bool-constructor to std::string one).
    value(bool);                                                 // Constructs boolean node
    value(double);                                               // Constructs numeric value
    value(float v) : value((double)v) {}                         // Constructs numeric value
    value(short v) : value((double)v) {}                         // Constructs numeric value
    value(int v) : value((double)v) {}                           // Constructs numeric value
    value(long v) : value((double)v) {}                          // Constructs numeric value
    value(value_type type);                                      // Constructs any type of value wanted.
    //value(std::initializer_list<value>);                         // Constructs an array
    value(std::initializer_list<std::pair<std::string, value>>); // Constructs an object 

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
    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;

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

    // Serialization
    std::string serialize() const;

    // Object-related stuff
    bool has(const std::string&) const;
    value& operator[](const std::string&);
    void remove(const std::string&);

    using object_entry = std::pair<const std::string&, value&>;
    struct object_iterator : public std::iterator<std::forward_iterator_tag, object_entry> {
      object_iterator(const object_iterator& other);
      object_iterator(const std::unordered_map<std::string, std::unique_ptr<value>>::const_iterator&);
      ~object_iterator();
      object_iterator& operator++();
      object_iterator operator++(int) {object_iterator res = *this; ++(*this); return res;}
      bool operator==(object_iterator other) const;
      bool operator!=(object_iterator other) const;
      reference operator*() const;
    private:
      std::unordered_map<std::string, std::unique_ptr<value>>::const_iterator source;
      mutable std::unique_ptr<object_entry> value_reference;
    };
    object_iterator begin() const;
    object_iterator end() const;

    struct array_iterator : public std::iterator<std::forward_iterator_tag, value> {
      array_iterator(const array_iterator& other);
      array_iterator(const std::vector<std::unique_ptr<value>>::const_iterator&);
      ~array_iterator();
      array_iterator& operator++();
      array_iterator operator++(int) {array_iterator res = *this; ++(*this); return res;}
      bool operator==(array_iterator other) const;
      bool operator!=(array_iterator other) const;
      reference operator*() const;
    private:
      std::vector<std::unique_ptr<value>>::const_iterator source;
    };
    array_iterator abegin();
    array_iterator aend();

    // Array-related stuff
    value& operator[](size_t);
    size_t push(value);
    size_t remove(size_t);

    size_t size() const; // For object and arrays returns the number of entries.
    bool empty() const;  // Returns true if object/array is empty 

    friend void swap(value& lhs, value& rhs);
  private:
    void from(const value&);
    void release();
    // Either a Boost variant or C++17 variant here is more proper.
    value_type type;
    union {
      double                                                  number;
      std::string                                             string;
      bool                                                    boolean;
      std::unordered_map<std::string, std::unique_ptr<value>> object;
      std::vector<std::unique_ptr<value>>                     array;
    };
  };

  std::ostream& operator<<(std::ostream&, const value&);
}

#endif
