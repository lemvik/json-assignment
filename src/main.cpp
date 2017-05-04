#include "json_sa.h"
#include <sstream>
#include <iostream>

int main(int, char*[]) {
  std::stringstream ss("asdfa;dg");
  std::cout << "Result: " << json::simple::literal(ss, "null") << "\n";

  return 0;
}
