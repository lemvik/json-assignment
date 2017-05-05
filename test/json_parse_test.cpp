#include <boost/test/unit_test.hpp>

#include "json.h"
#include "json_parser.h"
#include <memory>

BOOST_AUTO_TEST_SUITE(JSONParser)

BOOST_AUTO_TEST_CASE(ParseNull) {
  std::unique_ptr<json::json_node> node = json::parser::parse("null");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::null, node->type());
}

BOOST_AUTO_TEST_CASE(ParseString) {
  std::unique_ptr<json::json_node> node = json::parser::parse("\"Some string\"");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::string, node->type());
  BOOST_CHECK_EQUAL("Some string", node->string());
}

BOOST_AUTO_TEST_CASE(ParseNumber) {
  std::unique_ptr<json::json_node> node = json::parser::parse("8.128");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::number, node->type());
  BOOST_CHECK_EQUAL(8.128, node->number());
}

BOOST_AUTO_TEST_CASE(ParseBoolean) {
  std::unique_ptr<json::json_node> true_node = json::parser::parse("true");
  std::unique_ptr<json::json_node> false_node = json::parser::parse("false");

  BOOST_CHECK(true_node);
  BOOST_CHECK(false_node);
  BOOST_CHECK_EQUAL(json::json_value_type::boolean, true_node->type());
  BOOST_CHECK_EQUAL(json::json_value_type::boolean, false_node->type());
  BOOST_CHECK(*true_node);
  BOOST_CHECK(!*false_node);
}

BOOST_AUTO_TEST_CASE(ParseSimpleObject) {
  std::unique_ptr<json::json_node> node = json::parser::parse("{\"key\":\"value\"}");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::object, node->type());
  BOOST_CHECK((*node)["key"]);
  BOOST_CHECK_EQUAL(json::json_value_type::string, (*node)["key"]->type());
  BOOST_CHECK_EQUAL("value", (*node)["key"]->string());
}

BOOST_AUTO_TEST_CASE(ParseComplexObject) {
  std::unique_ptr<json::json_node> node = json::parser::parse("{\"key\":{\"subkey\":1}}");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::object, node->type());
  BOOST_CHECK((*node)["key"]);
  BOOST_CHECK_EQUAL(json::json_value_type::object, (*node)["key"]->type());
  BOOST_CHECK((*((*node)["key"]))["subkey"]);
  BOOST_CHECK_EQUAL(1, (*((*node)["key"]))["subkey"]->number());
}

BOOST_AUTO_TEST_CASE(ParseSimpleArray) {
  std::unique_ptr<json::json_node> node = json::parser::parse("[1,2]");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::array, node->type());
  BOOST_CHECK((*node)[0]);
  BOOST_CHECK_EQUAL(json::json_value_type::number, (*node)[0]->type());
  BOOST_CHECK_EQUAL(1, (*node)[0]->number());
  BOOST_CHECK((*node)[1]);
  BOOST_CHECK_EQUAL(json::json_value_type::number, (*node)[1]->type());
  BOOST_CHECK_EQUAL(2, (*node)[1]->number());
}

BOOST_AUTO_TEST_CASE(ParseArrayObject) {
  std::unique_ptr<json::json_node> node = json::parser::parse("[1,{\"key\":2}]");

  BOOST_CHECK(node);
  BOOST_CHECK_EQUAL(json::json_value_type::array, node->type());
  BOOST_CHECK((*node)[0]);
  BOOST_CHECK_EQUAL(json::json_value_type::number, (*node)[0]->type());
  BOOST_CHECK_EQUAL(1, (*node)[0]->number());
  BOOST_CHECK((*node)[1]);
  BOOST_CHECK_EQUAL(json::json_value_type::object, (*node)[1]->type());
  BOOST_CHECK((*((*node)[1]))["key"]);
  BOOST_CHECK_EQUAL(json::json_value_type::number, (*((*node)[1]))["key"]->type());
  BOOST_CHECK_EQUAL(2, (*((*node)[1]))["key"]->number());
}

BOOST_AUTO_TEST_SUITE_END()
