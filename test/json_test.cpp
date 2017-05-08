#include <boost/test/unit_test.hpp>

#include "json.h"
#include <string>
#include <sstream>
#include <iostream>

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

  object["keyA"] = json::value(json::value_type::object);
  object["keyA"]["subkey"] = 1.0;

  BOOST_CHECK_EQUAL(json::value_type::object, object["keyA"].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, object["keyA"]["subkey"].get_type());
  BOOST_CHECK_EQUAL(1.0, object["keyA"]["subkey"].as_number());
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

BOOST_AUTO_TEST_SUITE_END() 
