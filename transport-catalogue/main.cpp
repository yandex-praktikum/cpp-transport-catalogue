#include <iostream>


#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"

int main() {
  json_input::Parser parser;
  TransportCatalogue catalogue;
  auto document = json::Load(std::cin);
  auto root = document.GetRoot().AsDict();
  auto input = root["base_requests"].AsArray();
  auto graph_info = root["routing_settings"].AsDict();
  parser.LoadInputQueries(input, catalogue);


  TransportRouter router(catalogue);
  parser.LoadRoutQuery(graph_info, router);

  router.ProcessGraph();
  auto output = root["stat_requests"].AsArray();
  auto rend_info = root["render_settings"].AsDict();



  auto ans = json_output::LoadOutputQueries(output, catalogue, rend_info, router);
  json::Print(json::Document{ans}, std::cout);


}
