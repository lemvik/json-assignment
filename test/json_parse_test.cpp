#include <boost/test/unit_test.hpp>

#include "json.h"
#include "json_parser.h"
#include <memory>

BOOST_AUTO_TEST_SUITE(JSONParser)

BOOST_AUTO_TEST_CASE(ParseAndWriteNull) {
  auto node = json::parser::parse("null");

  BOOST_CHECK_EQUAL(json::value_type::null, node.get_type());
  BOOST_CHECK_EQUAL("null", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteString) {
  auto node = json::parser::parse("\"Some string\"");

  BOOST_CHECK_EQUAL(json::value_type::string, node.get_type());
  BOOST_CHECK_EQUAL("\"Some string\"", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteNumber) {
  auto node = json::parser::parse("8.128");

  BOOST_CHECK_EQUAL(json::value_type::number, node.get_type());
  BOOST_CHECK_EQUAL(8.128, node.as_number());
  BOOST_CHECK_EQUAL("8.128", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteBoolean) {
  auto true_node = json::parser::parse("true");
  auto false_node = json::parser::parse("false");

  BOOST_CHECK_EQUAL(json::value_type::boolean, true_node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::boolean, false_node.get_type());
  BOOST_CHECK(true_node.as_boolean());
  BOOST_CHECK(!false_node.as_boolean());
  BOOST_CHECK_EQUAL("true", true_node.serialize());
  BOOST_CHECK_EQUAL("false", false_node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteSimpleObject) {
  auto node = json::parser::parse("{\"key\":\"value\"}");

  BOOST_CHECK_EQUAL(json::value_type::object, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::string, node["key"].get_type());
  BOOST_CHECK_EQUAL("value", node["key"].as_string());
  BOOST_CHECK_EQUAL("{\"key\":\"value\"}", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteComplexObject) {
  auto node = json::parser::parse("{\"key\":{\"subkey\":1}}");

  BOOST_CHECK_EQUAL(json::value_type::object, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::object, node["key"].get_type());
  BOOST_CHECK_EQUAL(1, node["key"]["subkey"].as_number());
  BOOST_CHECK_EQUAL("{\"key\":{\"subkey\":1}}", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseSimpleArray) {
  auto node = json::parser::parse("[1,2]");

  BOOST_CHECK_EQUAL(json::value_type::array, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[0].get_type());
  BOOST_CHECK_EQUAL(1, node[0].as_number());
  BOOST_CHECK_EQUAL(json::value_type::number, node[1].get_type());
  BOOST_CHECK_EQUAL(2, node[1].as_number());
  BOOST_CHECK_EQUAL("[1,2]", node.serialize());
}

BOOST_AUTO_TEST_CASE(ParseAndWriteArrayObject) {
  auto node = json::parser::parse("[1,{\"key\":2}]");

  BOOST_CHECK_EQUAL(json::value_type::array, node.get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[0].get_type());
  BOOST_CHECK_EQUAL(1, node[0].as_number());
  BOOST_CHECK_EQUAL(json::value_type::object, node[1].get_type());
  BOOST_CHECK_EQUAL(json::value_type::number, node[1]["key"].get_type());
  BOOST_CHECK_EQUAL(2, node[1]["key"].as_number());
  BOOST_CHECK_EQUAL("[1,{\"key\":2}]", node.serialize());
}

BOOST_AUTO_TEST_SUITE_END()
