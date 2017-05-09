#include <boost/test/unit_test.hpp>

#include "json.h"
#include <string>
#include <ostream>
#include <sstream>
#include <iostream>
#include <unordered_set>

// Following is thanks to this explanation: http://stackoverflow.com/a/18817428
struct boost_compatible_unordered_set : public std::unordered_set<std::string> {
  boost_compatible_unordered_set(std::initializer_list<std::string> lst) : std::unordered_set<std::string>(lst) {}
};

std::ostream& operator<<(std::ostream& os, const boost_compatible_unordered_set& set) {
  os << "set:[";
  for (auto& el : set) {
    os << el;
  }
  os << "]";
  return os;
}


BOOST_AUTO_TEST_SUITE(JsonAPItest)

BOOST_AUTO_TEST_CASE(Constructors) {
  json::value val;

  BOOST_CHECK_EQUAL(json::value_type::null, val.get_type());

  json::value valS{"str"};

  BOOST_CHECK_EQUAL(json::value_type::string, valS.get_type());

  json::value valT{true};

  BOOST_CHECK_EQUAL(json::value_type::boolean, valT.get_type());

  json::value valF{false};

  BOOST_CHECK_EQUAL(json::value_type::boolean, valF.get_type());

  json::value valD{0.0};

  BOOST_CHECK_EQUAL(json::value_type::number, valD.get_type());

  json::value valO(json::value_type::object);

  BOOST_CHECK_EQUAL(json::value_type::object, valO.get_type());

  json::value valA(json::value_type::array);

  BOOST_CHECK_EQUAL(json::value_type::array, valA.get_type());
}

BOOST_AUTO_TEST_CASE(Assignments) {
  json::value val;

  BOOST_CHECK_EQUAL(json::value_type::null, val.get_type());

  val = "str";

  BOOST_CHECK_EQUAL(json::value_type::string, val.get_type());

  val = true;

  BOOST_CHECK_EQUAL(json::value_type::boolean, val.get_type());

  val = false;

  BOOST_CHECK_EQUAL(json::value_type::boolean, val.get_type());

  val = 0.0;

  BOOST_CHECK_EQUAL(json::value_type::number, val.get_type());

  val = nullptr;

  BOOST_CHECK_EQUAL(json::value_type::null, val.get_type());
}

BOOST_AUTO_TEST_CASE(ObjectStuff) {
  json::value object(json::value_type::object);

  object["key"] = "value";

  BOOST_CHECK_EQUAL(json::value_type::string, object["key"].get_type());
  BOOST_CHECK_EQUAL("value", object["key"].as_string());

  // Test copy-construction.
  {
    json::value another_object = object;
    BOOST_CHECK(&another_object != &object);
    BOOST_CHECK_EQUAL(another_object["key"].as_string(), object["key"].as_string());
    BOOST_CHECK(&(another_object["key"]) != &(object["key"]));
  }

  // Check that we didn't break anything.
  BOOST_CHECK_EQUAL(json::value_type::string, object["key"].get_type());
  BOOST_CHECK_EQUAL("value", object["key"].as_string());

  object["keyA"] = json::value(json::value_type::object);
  object["keyA"]["subkey"] = 1.0;

  BOOST_CHECK_EQUAL(json::value_type::object, object["keyA"].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, object["keyA"]["subkey"].get_type());
  BOOST_CHECK_EQUAL(1.0, object["keyA"]["subkey"].as_number());



}


BOOST_AUTO_TEST_CASE(ObjectIteration) {
  json::value object(json::value_type::object);

  object["keyA"] = "valueA";
  object["keyB"] = "valueB";
  object["keyC"] = "valueC";

  boost_compatible_unordered_set expected{std::string("[keyA:valueA]"), std::string("[keyB:valueB]"), std::string("[keyC:valueC]")};
  boost_compatible_unordered_set actual{};

  for (auto it = object.begin(); it != object.end(); ++it) {
    std::stringstream ss;
    ss << "[" << (*it).first << ":" << (*it).second.as_string() << "]";
    actual.insert(ss.str());
  }

  BOOST_CHECK_EQUAL(expected, actual);

  actual.clear();

  for (auto& p : object) {
    std::stringstream ss;
    ss << "[" << p.first << ":" << p.second.as_string() << "]";
    actual.insert(ss.str());
  }

  BOOST_CHECK_EQUAL(expected, actual);

  for (auto& p : object) {
    if (p.first == "keyA") {
      p.second = 1.0;
    } else if (p.first == "keyB") {
      p.second = true; 
    } else if (p.first == "keyC") {
      p.second = "last";
    }
  }

  BOOST_CHECK_EQUAL(1.0, object["keyA"].as_number());
  BOOST_CHECK_EQUAL(true, object["keyB"].as_boolean());
  BOOST_CHECK_EQUAL("last", object["keyC"].as_string());
}

BOOST_AUTO_TEST_CASE(ArrayStuff) {
  json::value array(json::value_type::array);

  array.push(1.0);
  array.push("str");
  array.push(json::value(true)); // Here for some "reason" compiler prefers non-explicit constructor value(double)
                                 // Force the fu..er to push boolean.

  BOOST_CHECK_EQUAL(json::value_type::number,  array[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::string,  array[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array[2].get_type());
}

BOOST_AUTO_TEST_CASE(ArrayIteration) {
  json::value object(json::value_type::array);

  object.push("valueA");
  object.push("valueB");
  object.push("valueC");

  std::stringstream ss;

  for (auto it = object.abegin(); it != object.aend(); ++it) {
    ss << "[" << (*it).as_string() << "]";
  }

  BOOST_CHECK_EQUAL("[valueA][valueB][valueC]", ss.str());
}

BOOST_AUTO_TEST_SUITE_END() 
