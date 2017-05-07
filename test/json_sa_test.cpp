#include <boost/test/unit_test.hpp>

#include "json_sa.h"
#include <string>
#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(JSONTokenizer)

enum class stopper { null, object_open, array_end, none };

struct test_callback : public json::simple::token_callback {
  stopper flag;
  bool need_more = true;
  std::stringstream buffer;

  test_callback(stopper f) : flag(f) {}

  void json_start() override {
    buffer << "[start]";
  }

  void json_end() override {
    buffer << "[end]";
  }

  void json_null() override {
    need_more = !(flag == stopper::null);
    buffer << "[null]";
  }

  void json_string(const std::string& content) override {
    buffer << "[string:" << content << "]";
  }

  void json_number(double n) override {
    buffer << "[number:" << n << "]";
  }

  void json_boolean(bool b) override {
    buffer << "[boolean:" << (b ? "true" : "false") << "]";
  }

  void json_error(const std::string& error) override {
    std::cerr << "Got error: " << error << "\n";
    buffer << "[error]";
  }

  void json_array_starts() override {
    buffer << "[arr::start]";
  }
  void json_array_ends() override {
    need_more = !(flag == stopper::array_end);
    buffer << "[arr::end]";
  }

  void json_comma() override {
    buffer << "[comma]";
  }
  void json_colon() override {
    buffer << "[colon]";
  }

  void json_object_starts() override {
    need_more = !(flag == stopper::object_open);
    buffer << "[object::start]";
  }
  void json_object_ends() override {
    buffer << "[object::end]";
  }

  bool need_more_json() override {
    return need_more;
  }
};

BOOST_AUTO_TEST_CASE(ReadNull) {
  test_callback callback(stopper::null);

  run_tokenizer("null", callback);

  BOOST_CHECK_EQUAL("[start][null][end]", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadGarbled) {
  test_callback callback(stopper::object_open);

  run_tokenizer("[,:null}]{", callback);

  BOOST_CHECK_EQUAL("[start][arr::start][comma][colon][null][object::end][arr::end][object::start][end]", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadNumber) {
  test_callback callback(stopper::array_end);

  run_tokenizer("[0.1234]", callback);

  BOOST_CHECK_EQUAL("[start][arr::start][number:0.1234][arr::end][end]", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadString) {
  test_callback callback(stopper::array_end);

  run_tokenizer(R"%(["ololo-trololo\"somestuff"])%", callback);

  BOOST_CHECK_EQUAL(R"%([start][arr::start][string:ololo-trololo\"somestuff][arr::end][end])%", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadNumberFail) {
  test_callback callback(stopper::array_end);

  run_tokenizer("[0.1234asdf]", callback);

  BOOST_CHECK_EQUAL("[start][arr::start][number:0.1234][error]", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadArray) {
  test_callback callback(stopper::array_end);

  run_tokenizer(R"%([null,   "ololo-trololo\"somestuff", true])%", callback);

  BOOST_CHECK_EQUAL(R"%([start][arr::start][null][comma][string:ololo-trololo\"somestuff][comma][boolean:true][arr::end][end])%", callback.buffer.str());
}

BOOST_AUTO_TEST_CASE(ReadMangled) {
  test_callback callback(stopper::null);

  run_tokenizer("as;dlkjf;a", callback);

  BOOST_CHECK_EQUAL("[start][error]", callback.buffer.str());
}

BOOST_AUTO_TEST_SUITE_END() 
