#include <iostream>
#include <ostream>

#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

int main() {
  json_input::Parser parser;
  TransportCatalogue catalogue;
  std::ofstream out;
  out.open("output.svg");
  auto document = json::Load(std::cin);
  auto root = document.GetRoot().AsMap();
  auto input = root["base_requests"].AsArray();
  parser.LoadInputQueries(input, catalogue);
  auto output = root["stat_requests"].AsArray();
  auto rend_info = root["render_settings"].AsMap();

  auto ans = json_output::LoadOutputQueries(output, catalogue, rend_info);
  json::Print(json::Document{ans}, std::cout);

}
