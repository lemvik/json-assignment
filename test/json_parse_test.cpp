#include <boost/test/unit_test.hpp>

#include "json.h"
#include "json_parser.h"
#include <memory>

BOOST_AUTO_TEST_SUITE(JSONParser)

BOOST_AUTO_TEST_CASE(ParseNull) {
  auto node = json::parser::parse("null");

  BOOST_CHECK_EQUAL(json::value_type::null, node.get_type());
}

BOOST_AUTO_TEST_CASE(ParseString) {
  auto node = json::parser::parse("\"Some string\"");

  BOOST_CHECK_EQUAL(json::value_type::string, node.get_type());
  BOOST_CHECK_EQUAL("Some string", node.as_string());
}

BOOST_AUTO_TEST_CASE(ParseNumber) {
  auto node = json::parser::parse("8.128");

  BOOST_CHECK_EQUAL(json::value_type::number, node.get_type());
  BOOST_CHECK_EQUAL(8.128, node.as_number());
}

BOOST_AUTO_TEST_CASE(ParseBoolean) {
  auto true_node = json::parser::parse("true");
  auto false_node = json::parser::parse("false");

  BOOST_CHECK_EQUAL(json::value_type::boolean, true_node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, false_node.get_type());
  BOOST_CHECK(true_node.as_boolean());
  BOOST_CHECK(!false_node.as_boolean());
}

BOOST_AUTO_TEST_CASE(ParseSimpleObject) {
  auto node = json::parser::parse("{\"key\":\"value\"}");

  BOOST_CHECK_EQUAL(json::value_type::object, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::string, node["key"].get_type());
  BOOST_CHECK_EQUAL("value", node["key"].as_string());
}

BOOST_AUTO_TEST_CASE(ParseComplexObject) {
  auto node = json::parser::parse("{\"key\":{\"subkey\":1}}");

  BOOST_CHECK_EQUAL(json::value_type::object, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::object, node["key"].get_type());
  BOOST_CHECK_EQUAL(1, node["key"]["subkey"].as_number());
}

BOOST_AUTO_TEST_CASE(ParseSimpleArray) {
  auto node = json::parser::parse("[1,2]");

  BOOST_CHECK_EQUAL(json::value_type::array, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[0].get_type());
  BOOST_CHECK_EQUAL(1, node[0].as_number());
  BOOST_CHECK_EQUAL(json::value_type::number, node[1].get_type());
  BOOST_CHECK_EQUAL(2, node[1].as_number());
}

BOOST_AUTO_TEST_CASE(ParseArrayObject) {
  auto node = json::parser::parse("[1,{\"key\":2}]");

  BOOST_CHECK_EQUAL(json::value_type::array, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[0].get_type());
  BOOST_CHECK_EQUAL(1, node[0].as_number());
  BOOST_CHECK_EQUAL(json::value_type::object, node[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[1]["key"].get_type());
  BOOST_CHECK_EQUAL(2, node[1]["key"].as_number());
}

BOOST_AUTO_TEST_SUITE_END()
