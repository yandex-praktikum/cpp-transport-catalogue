#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json.h"

namespace json_input {

class Parser {
 public:

  void LoadInputQueries(const json::Array & list, TransportCatalogue& catalogue);


 private:
  void ParseQuery(const json::Dict& query);
  void ParseStopQuery(const json::Dict& stop, TransportCatalogue& catalogue);
  void ParseBusQuery(const json::Dict& bus, TransportCatalogue& catalogue);
  void ParseAllQueries(TransportCatalogue& catalogue);
  void ParseCicleBusQuery(const json::Dict& bus, TransportCatalogue& catalogue);
  void ParseDistances(TransportCatalogue& catalogue);

  std::deque<json::Node> deque_queries;
  std::list<std::string> all_queries;
  std::map<std::string_view , std::vector<std::pair<std::string_view , int>>> dist_;
};

}

namespace json_output {
void WriteBus(const json::Dict & bus,  TransportCatalogue& catalogue, json::Array& ans);


void WriteStop(const json::Dict& stop, TransportCatalogue& catalogue,json::Array& ans);

void WriteMap(const json::Dict& stop, TransportCatalogue& catalogue, json::Array& ans, const json::Dict& settings);

json::Array LoadOutputQueries(const json::Array& list,  TransportCatalogue& catalogue, const json::Dict& settings);

} // namespace json_output

