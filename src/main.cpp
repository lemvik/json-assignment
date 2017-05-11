#include "json.h"
#include "json_parser.h"
#include <iostream>
#include <fstream>

int main(int argc, char* args[]) {
  if (argc < 2) {
    std::cerr << "Please provide file name as first argument.";
    return -1;
  }

  try {
    std::ifstream ifile(args[1], std::ios::in);
    json::value input = json::parser::parse(ifile);

    for (auto it = input.abegin(); it != input.aend(); ++it) {
      std::cout << (*it)["_id"].as_string() << "\n";
    }
  } catch (const json::json_error& err) {
    std::cerr << err.what() << "\n";
    return -1;
  }

  return 0;
}
