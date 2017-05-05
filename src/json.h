#ifndef _JSON_H_
#define _JSON_H_

#include <unordered_map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <string>
#include <memory>

namespace json {
  enum class json_value_type { null, number, boolean, string, object, array };

  std::ostream& operator<<(std::ostream& os, const json_value_type& val); 

  class json_error : public std::runtime_error {
  public:
    json_error(const std::string& reason) : std::runtime_error(reason) {}
  };

  class json_node {
  private:
    json_value_type value_type;
  protected:
    json_node(json_value_type _type) : value_type(_type) {}
  public:
    virtual ~json_node() {}

    virtual std::string& string() = 0;
    virtual double&      number() = 0;

    virtual explicit operator bool() const = 0;

    json_value_type type() const {
      return value_type;
    }

    virtual std::shared_ptr<json_node>& operator[](const std::string& key) = 0;
    virtual std::shared_ptr<json_node>& operator[](unsigned int) = 0;
  };

  class json_string_node : public json_node {
    std::string contents;
  public:
    json_string_node(std::string _content) : json_node(json_value_type::string), contents(_content) {}

    std::string& string() override {
      return contents;
    }

    double& number() override {
      throw json_error("Unable to produce number from string node.");
    }

    explicit operator bool() const override {
      return !contents.empty(); // A bit of a cheat here, as in JS "0" string is also 'false'
    }

    std::shared_ptr<json_node>& operator[](const std::string&) override {
      throw json_error("Unable to apply indexing [key] operator to string node.");
    }

    std::shared_ptr<json_node>& operator[](unsigned int) override {
      throw json_error("Unable to apply indexing [int] operator to string node.");
    }
  };

  class json_numeric_node : public json_node {
    double contents;
  public:
    json_numeric_node(double _contents) : json_node(json_value_type::number), contents(_contents) {}

    std::string& string() override {
      throw json_error("Unable to produce string from number node.");
    }

    double& number() override {
      return contents;
    }

    explicit operator bool() const override {
      return contents != 0;
    }

    std::shared_ptr<json_node>& operator[](const std::string&) override {
      throw json_error("Unable to apply indexing [key] operator to numeric node.");
    }

    std::shared_ptr<json_node>& operator[](unsigned int) override {
      throw json_error("Unable to apply indexing [int] operator to numeric node.");
    }
  };

  class json_boolean_node : public json_node {
    bool value;
  public:
    explicit json_boolean_node(bool _value) : json_node(json_value_type::boolean), value(_value) {}

    std::string& string() override {
      throw json_error("Unable to produce string from boolean node.");
    }

    double& number() override {
      throw json_error("Unable to produce number from boolean node.");
    }

    explicit operator bool() const override {
      return value;
    }

    std::shared_ptr<json_node>& operator[](const std::string&) override {
      throw json_error("Unable to apply indexing [key] operator to boolean node.");
    }

    std::shared_ptr<json_node>& operator[](unsigned int) override {
      throw json_error("Unable to apply indexing [int] operator to boolean node.");
    }
  };

  class json_null_node : public json_node {
  public:
    json_null_node() : json_node(json_value_type::null) {}

    std::string& string() override {
      throw json_error("Unable to produce string from null node.");
    }

    double& number() override {
      throw json_error("Unable to produce number from null node.");
    }

    explicit operator bool() const override {
      return false;
    }

    std::shared_ptr<json_node>& operator[](const std::string&) override {
      throw json_error("Unable to apply indexing [key] operator to null node.");
    }

    std::shared_ptr<json_node>& operator[](unsigned int) override {
      throw json_error("Unable to apply indexing [int] operator to null node.");
    }
  };

  class json_object_node : public json_node {
    std::unordered_map<std::string, std::shared_ptr<json_node>> childs;

  public:
    json_object_node() : json_node(json_value_type::object), childs() {}
   
    std::string& string() override {
      throw json_error("Unable to produce string from object node.");
    }

    double& number() override {
      throw json_error("Unable to produce number from object node.");
    }

    explicit operator bool() const override {
      return !childs.empty();
    }

    std::shared_ptr<json_node>& operator[](const std::string& key) override {
      return childs[key];
    }

    std::shared_ptr<json_node>& operator[](unsigned int) override {
      throw json_error("Unable to apply indexing [int] operator to object node.");
    }
  };

  class json_array_node : public json_node {
    std::vector<std::shared_ptr<json_node>> elements;
  public:
    json_array_node() : json_node(json_value_type::array), elements() {}

    std::string& string() override {
      throw json_error("Unable to produce string from array node.");
    }

    double& number() override {
      throw json_error("Unable to produce number from array node.");
    }

    explicit operator bool() const override {
      return !elements.empty();
    }

    std::shared_ptr<json_node>& operator[](const std::string&) override {
      throw json_error("Unable to apply indexing [key] operator to array node.");
    }

    std::shared_ptr<json_node>& operator[](unsigned int index) override {
      if (index < elements.size()) {
        return elements[index];
      }

      throw json_error("Unable to apply indexing [int] operator to object node.");
    }

    void push(std::shared_ptr<json_node> element) {
      elements.push_back(element);
    }
  };
}

#endif
