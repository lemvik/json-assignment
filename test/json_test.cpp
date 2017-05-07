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

BOOST_AUTO_TEST_SUITE_END() 
