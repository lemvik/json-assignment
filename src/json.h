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
  // This operator overload is to simplify debbugging/testing mostly.
  std::ostream& operator<<(std::ostream& os, const value_type& val);

  // Error to signal if anything goes awry.
  struct json_error : public std::runtime_error {
    json_error(const std::string& reason) : std::runtime_error(reason) {}
  };

  // Forward declarations for array_value and object_value
  class array_value;
  class object_value;

  // The structure that encapsulates JSON value. Relies on runtime checks to
  // check validity of operations. Have two proxies - for objects and for arrays operations.
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
    //value(std::initializer_list<value>);                       // Constructs an array
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
    value& operator=(std::nullptr_t);     // Overload of null assignment (to be able to write obj["key"] = nullptr a-la JavaScript)

    // Comparison operators
    bool operator==(const value&)   const;
    bool operator!=(const value&)   const;
    bool operator==(std::nullptr_t) const; // This and following methods are to simplify comparison frequently
    bool operator!=(std::nullptr_t) const; // encoutered in JS code (obj["key"] == null(ptr))

    // Type checking
    bool       is_null()    const;
    bool       is_string()  const;
    bool       is_number()  const;
    bool       is_boolean() const;
    bool       is_object()  const;
    bool       is_array()   const;
    value_type get_type()   const;

    // Retrieving values
    std::string as_string()  const;
    double      as_number()  const;
    bool        as_boolean() const;

    // Serialization - write a JSON representation of the value.
    std::string serialize() const;

    // Object-related stuff
    // Returns true if object contains a key.
    bool has(const std::string&) const;
    // Returns reference to value stored in given key. Mimics the behaviour of std::unordered_map[key]
    // in that it returns either a stored value or associates default value with key and returns that.
    value& operator[](const std::string&);
    // Removed key association from object. If no key exists - does nothing.
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

    // Array-related stuff

    // Returns reference to a value stored in JSON array. Mimics behaviour of
    // std::vector.at(): if index is out of bounds, raises an exception of type out_of_range
    value& operator[](size_t);
    // Pushes value into the array, returning size of the array (and the index of pushed element, coincidentally)
    size_t push(value);
    // Removes the element at given index, returning new size of array. If index is out
    // of array's bounds, throws an out_of_range exception.
    size_t remove(size_t);

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

    // For object and arrays returns the number of entries.
    size_t size() const;
    // Returns true if object/array is empty
    bool empty() const;

    friend void swap(value& lhs, value& rhs);
    friend class array_value;
    friend class object_value;

    // Following two methods return views to this value that is only
    // valid while the value exists. This allows us to avoid copy and have
    // these objects cost little.
    array_value  as_array();
    object_value as_object();
  private:
    // Populates this instance from another one.
    void from(const value&);
    // Releases currently held value. Sets type to null. noexcept explained in .cpp.
    void release() noexcept;
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

  // This class provides array-specific interface to aleviate some runtime checks and provide compatibility
  // with STL. This is actually a facade for plain value, but with runtime checks removed and more appropriate method names.
  // The lifetime of the array_value is the same as of the value it was constructed with.
  // NOTE: this wrapper becomes invalidated if underlying value changes it's type. This is checked via
  // assertions and can fail unpredictably in release builds.
  class array_value {
    value& wrapped_value;
    array_value(value& _value);
    friend struct value;

    // Copies are prohibited because we want to
    // avoid creating copies that are hard to track and prone to
    // leave dangling references if wrapped value goes away.
    array_value(const array_value&)            = delete;
    array_value& operator=(const array_value&) = delete;
  public:
    // Moves are allowed to facilitate returning from methods and passing into them
    array_value(array_value&&);
    array_value& operator=(array_value&&);
    // Docs for methods below are the same as for value methods.
    value& operator[](size_t);
    size_t push(value);
    size_t remove(size_t);
    size_t size() const;
    bool   empty() const;
    value::array_iterator begin();
    value::array_iterator end();
  };

  // This class provides object-specific interface to aleviate some runtime checks and provide compatibility
  // with STL. This is actually a facade for plain value, but with runtime checks removed and more appropriate method names.
  // The lifetime of the object_value is the same as of the value it was constructed with.
  // NOTE: this wrapper becomes invalidated if underlying value changes it's type. This is checked via
  // assertions and can fail unpredictably in release builds.
  class object_value {
    value& wrapped_value;
    object_value(value& _value);
    friend struct value;

    // Copy and moves are prohibited because we want to
    // avoid creating copies that are hard to track and prone to
    // leave dangling references if wrapped value goes away.
    object_value(const object_value&)            = delete;
    object_value& operator=(const object_value&) = delete;
  public:
    // Moves are allowed to facilitate returning from methods and passing into them
    object_value(object_value&&);
    object_value& operator=(object_value&&);
    // Docs for methods below are the same as for value methods.
    bool   has(const std::string&) const;
    value& operator[](const std::string&);
    void   remove(const std::string&);
    size_t size() const;
    bool   empty() const;
    value::object_iterator begin();
    value::object_iterator end();
  };

  // Overload for outputting to stream (internally works via serialize).
  std::ostream& operator<<(std::ostream&, const value&);
}

#endif
