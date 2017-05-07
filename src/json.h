#ifndef _JSON_H_
#define _JSON_H_

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

  class value {
    value_type type;
    value(value_type);
  public:
    // Constructor and assignment stuff
    value(const value&);
    value(value&&);
    value& operator=(const value&);
    value& operator=(value&&);

    // Custom constructors from primitives
    value();                   // Constructs null value
    value(const std::string&); // Constructs string value
    value(double);             // Constructs numeric value
    explicit value(bool);      // Constructs boolean node

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

    using object_entry = std::pair<const std::string&, value&>;
    class object_iterator : public std::iterator<std::forward_iterator_tag, object_entry> {
    };
    object_iterator begin() const;
    object_iterator end() const;
    class const_object_iterator : public std::iterator<std::forward_iterator_tag, const object_entry> {
    };
    const_object_iterator cbegin() const;
    const_object_iterator cend() const;

    // Array-related stuff
    value& operator[](size_t);
    void push(value);

    friend void swap(value& lhs, value& rhs);
  };


  // class json_node {
  // private:
  //   json_value_type value_type;
  // protected:
  //   json_node(json_value_type _type) : value_type(_type) {}
  // public:
  //   virtual ~json_node() {}

  //   virtual std::string& string() = 0;
  //   virtual double&      number() = 0;

  //   virtual explicit operator bool() const = 0;

  //   json_value_type type() const {
  //     return value_type;
  //   }

  //   virtual std::shared_ptr<json_node>& operator[](const std::string& key) = 0;
  //   virtual std::shared_ptr<json_node>& operator[](unsigned int) = 0;
  // };

  // class json_string_node : public json_node {
  //   std::string contents;
  // public:
  //   json_string_node(std::string _content) : json_node(json_value_type::string), contents(_content) {}

  //   std::string& string() override {
  //     return contents;
  //   }

  //   double& number() override {
  //     throw json_error("Unable to produce number from string node.");
  //   }

  //   explicit operator bool() const override {
  //     return !contents.empty(); // A bit of a cheat here, as in JS "0" string is also 'false'
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string&) override {
  //     throw json_error("Unable to apply indexing [key] operator to string node.");
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int) override {
  //     throw json_error("Unable to apply indexing [int] operator to string node.");
  //   }
  // };

  // class json_numeric_node : public json_node {
  //   double contents;
  // public:
  //   json_numeric_node(double _contents) : json_node(json_value_type::number), contents(_contents) {}

  //   std::string& string() override {
  //     throw json_error("Unable to produce string from number node.");
  //   }

  //   double& number() override {
  //     return contents;
  //   }

  //   explicit operator bool() const override {
  //     return contents != 0;
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string&) override {
  //     throw json_error("Unable to apply indexing [key] operator to numeric node.");
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int) override {
  //     throw json_error("Unable to apply indexing [int] operator to numeric node.");
  //   }
  // };

  // class json_boolean_node : public json_node {
  //   bool value;
  // public:
  //   explicit json_boolean_node(bool _value) : json_node(json_value_type::boolean), value(_value) {}

  //   std::string& string() override {
  //     throw json_error("Unable to produce string from boolean node.");
  //   }

  //   double& number() override {
  //     throw json_error("Unable to produce number from boolean node.");
  //   }

  //   explicit operator bool() const override {
  //     return value;
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string&) override {
  //     throw json_error("Unable to apply indexing [key] operator to boolean node.");
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int) override {
  //     throw json_error("Unable to apply indexing [int] operator to boolean node.");
  //   }
  // };

  // class json_null_node : public json_node {
  // public:
  //   json_null_node() : json_node(json_value_type::null) {}

  //   std::string& string() override {
  //     throw json_error("Unable to produce string from null node.");
  //   }

  //   double& number() override {
  //     throw json_error("Unable to produce number from null node.");
  //   }

  //   explicit operator bool() const override {
  //     return false;
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string&) override {
  //     throw json_error("Unable to apply indexing [key] operator to null node.");
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int) override {
  //     throw json_error("Unable to apply indexing [int] operator to null node.");
  //   }
  // };

  // class json_object_node : public json_node {
  //   std::unordered_map<std::string, std::shared_ptr<json_node>> childs;

  // public:
  //   json_object_node() : json_node(json_value_type::object), childs() {}
   
  //   std::string& string() override {
  //     throw json_error("Unable to produce string from object node.");
  //   }

  //   double& number() override {
  //     throw json_error("Unable to produce number from object node.");
  //   }

  //   explicit operator bool() const override {
  //     return !childs.empty();
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string& key) override {
  //     return childs[key];
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int) override {
  //     throw json_error("Unable to apply indexing [int] operator to object node.");
  //   }
  // };

  // class json_array_node : public json_node {
  //   std::vector<std::shared_ptr<json_node>> elements;
  // public:
  //   json_array_node() : json_node(json_value_type::array), elements() {}

  //   std::string& string() override {
  //     throw json_error("Unable to produce string from array node.");
  //   }

  //   double& number() override {
  //     throw json_error("Unable to produce number from array node.");
  //   }

  //   explicit operator bool() const override {
  //     return !elements.empty();
  //   }

  //   std::shared_ptr<json_node>& operator[](const std::string&) override {
  //     throw json_error("Unable to apply indexing [key] operator to array node.");
  //   }

  //   std::shared_ptr<json_node>& operator[](unsigned int index) override {
  //     if (index < elements.size()) {
  //       return elements[index];
  //     }

  //     throw json_error("Unable to apply indexing [int] operator to object node.");
  //   }

  //   void push(std::shared_ptr<json_node> element) {
  //     elements.push_back(element);
  //   }
  // };
}

#endif
