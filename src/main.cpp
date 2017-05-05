#include "json.h"
#include "json_parser.h"
#include <iostream>

int main(int, char*[]) {
  std::shared_ptr<json::json_node> node = json::parser::parse("{\"key\":{\"subkey\":1}}");

  if (node) {
    std::cout << "String contents: " << node->string() << "\n";
  } else {
    std::cerr << "Failed to parse JSON\n";
  }

  return 0;
}
