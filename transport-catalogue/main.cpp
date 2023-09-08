#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

int main() {
  TransportCatalogue catalogue;

  input::Parser parser;

  parser.LoadInputQueries(std::cin, catalogue);

  output::LoadOutputQueries(std::cin, std::cout, catalogue);
}
