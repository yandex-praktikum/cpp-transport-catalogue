#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

int main() {
  //std::ifstream inputFile("input.txt");
  int N;
  std::cin >> N;
  //inputFile >> N;
  //std::ofstream file("output.txt");
  //inputFile.ignore();
  std::cin.ignore();
  std::string query;
  TransportCatalogue catalogue;
  input::Parser parser;
  for (int i = 0; i < N; ++i) {
    std::getline(std::cin, query);
    parser.ParseQuery(query);
  }
  parser.ParseAllQueries(catalogue);
  int K;
  std::cin >> K;
  std::cin.ignore();
  for (int i = 0; i < K; ++i) {
    std::getline(std::cin, query);
    output::ParseQuery(query, catalogue);
  }
  //inputFile.close();
  //file.close();
}
