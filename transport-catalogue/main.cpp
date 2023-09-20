#include <iostream>


#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

int main() {
  json_input::Parser parser;
  TransportCatalogue catalogue;
  auto document = json::Load(std::cin);
  auto root = document.GetRoot().AsDict();
  auto input = root["base_requests"].AsArray();
  parser.LoadInputQueries(input, catalogue);
  auto output = root["stat_requests"].AsArray();
  auto rend_info = root["render_settings"].AsDict();

  auto ans = json_output::LoadOutputQueries(output, catalogue, rend_info);
  json::Print(json::Document{ans}, std::cout);

}
