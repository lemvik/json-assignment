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

  json::value valS("str");

  BOOST_CHECK_EQUAL(json::value_type::string, valS.get_type());

  json::value valT(true);

  BOOST_CHECK_EQUAL(json::value_type::boolean, valT.get_type());

  json::value valF(false);

  BOOST_CHECK_EQUAL(json::value_type::boolean, valF.get_type());

  json::value valD(0.0);

  BOOST_CHECK_EQUAL(json::value_type::number, valD.get_type());

  json::value valShort((short)1);

  BOOST_CHECK_EQUAL(json::value_type::number, valShort.get_type());

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
  BOOST_CHECK_EQUAL("0", val.serialize());

  val = nullptr;

  BOOST_CHECK_EQUAL(json::value_type::null, val.get_type());
}

BOOST_AUTO_TEST_CASE(ObjectStuff) {
  json::value object(json::value_type::object);
  json::object_value object_proxy = object.as_object();

  object["key"] = "value";

  BOOST_CHECK_EQUAL(json::value_type::string, object["key"].get_type());
  BOOST_CHECK_EQUAL("value", object["key"].as_string());
  BOOST_CHECK_EQUAL(json::value_type::string, object_proxy["key"].get_type());
  BOOST_CHECK_EQUAL("value", object_proxy["key"].as_string());

  // Test copy-construction.
  {
    json::value another_object = object;
    BOOST_CHECK(&another_object != &object);
    BOOST_CHECK_EQUAL(another_object["key"].as_string(), object["key"].as_string());
    BOOST_CHECK(&(another_object["key"]) != &(object["key"]));
    another_object["key"] = "another_value";
    another_object["extra_key"] = 1.0;
    BOOST_CHECK_EQUAL("another_value", another_object["key"].as_string());
  }

  // Check that we didn't break anything.
  BOOST_CHECK_EQUAL(json::value_type::string, object["key"].get_type());
  BOOST_CHECK_EQUAL("value", object["key"].as_string());
  BOOST_CHECK(!object.has("extra_key"));
  BOOST_CHECK_EQUAL(json::value_type::string, object_proxy["key"].get_type());
  BOOST_CHECK_EQUAL("value", object_proxy["key"].as_string());
  BOOST_CHECK(!object_proxy.has("extra_key"));

  object["keyA"] = json::value(json::value_type::object);
  object["keyA"]["subkey"] = 1.0;

  BOOST_CHECK_EQUAL(json::value_type::object, object["keyA"].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, object["keyA"]["subkey"].get_type());
  BOOST_CHECK_EQUAL(1.0, object["keyA"]["subkey"].as_number());
  BOOST_CHECK_EQUAL(json::value_type::object, object_proxy["keyA"].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, object_proxy["keyA"]["subkey"].get_type());
  BOOST_CHECK_EQUAL(1.0, object_proxy["keyA"]["subkey"].as_number());

  BOOST_CHECK_EQUAL(2, object.size());
  BOOST_CHECK_EQUAL(2, object_proxy.size());

  object.remove("keyA");
  BOOST_CHECK(!object.has("keyA"));
  BOOST_CHECK_EQUAL(1, object.size());
  BOOST_CHECK(!object_proxy.has("keyA"));
  BOOST_CHECK_EQUAL(1, object_proxy.size());
}


BOOST_AUTO_TEST_CASE(ObjectIteration) {
  json::value object(json::value_type::object);

  object["keyA"] = "valueA";
  object["keyB"] = "valueB";
  object["keyC"] = "valueC";

  boost_compatible_unordered_set expected{std::string("[keyA:valueA]"), std::string("[keyB:valueB]"), std::string("[keyC:valueC]")};
  boost_compatible_unordered_set actual{};
  json::object_value object_value = object.as_object();

  for (auto it = object_value.begin(); it != object_value.end(); ++it) {
    std::stringstream ss;
    ss << "[" << (*it).first << ":" << (*it).second.as_string() << "]";
    actual.insert(ss.str());
  }

  BOOST_CHECK_EQUAL(expected, actual);

  actual.clear();

  for (auto& p : object_value) {
    std::stringstream ss;
    ss << "[" << p.first << ":" << p.second.as_string() << "]";
    actual.insert(ss.str());
  }

  BOOST_CHECK_EQUAL(expected, actual);

  for (auto& p : object_value) {
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

BOOST_AUTO_TEST_CASE(ObjectLiteral) {
  json::value object{{"keyA", 1.0}, {"keyB", true}, {"keyC", nullptr}};

  BOOST_CHECK_EQUAL(1.0, object["keyA"].as_number());
  BOOST_CHECK_EQUAL(true, object["keyB"].as_boolean());
  BOOST_CHECK(object["keyC"] == nullptr);
}


BOOST_AUTO_TEST_CASE(ArrayStuff) {
  json::value array(json::value_type::array);

  array.push(1.0);
  array.push("str");
  array.push(true); 

  json::array_value array_val = array.as_array();

  BOOST_CHECK_EQUAL(json::value_type::number,  array[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::string,  array[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array[2].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number,  array_val[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::string,  array_val[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array_val[2].get_type());

  array.remove(0);
  BOOST_CHECK_EQUAL(2, array.size());
  BOOST_CHECK_EQUAL(2, array_val.size());
  BOOST_CHECK_EQUAL(json::value_type::string,  array[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::string,  array_val[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array_val[1].get_type());

  array_val.remove(0);
  BOOST_CHECK_EQUAL(1, array.size());
  BOOST_CHECK_EQUAL(1, array_val.size());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array[0].get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, array_val[0].get_type());
}

BOOST_AUTO_TEST_CASE(ArrayIteration) {
  json::value array(json::value_type::array);

  array.push("valueA");
  array.push("valueB");
  array.push("valueC");

  std::stringstream ss;

  json::array_value array_value = array.as_array();

  for (auto it = array_value.begin(); it != array_value.end(); ++it) {
    ss << "[" << (*it).as_string() << "]";
  }

  BOOST_CHECK_EQUAL("[valueA][valueB][valueC]", ss.str());

  ss.str(std::string());

  for (auto& it : array_value) {
    ss << "[" << it.as_string() << "]";
  } 

  BOOST_CHECK_EQUAL("[valueA][valueB][valueC]", ss.str());
}

// BOOST_AUTO_TEST_CASE(ArrayLiteral) {
//   json::value object{"valueA", 1.0, false};

//   BOOST_CHECK_EQUAL("valueA", object[0].as_string());
//   BOOST_CHECK_EQUAL(1.0, object[1].as_number());
//   BOOST_CHECK_EQUAL(false, object[2].as_boolean());
// }

BOOST_AUTO_TEST_SUITE_END() 
