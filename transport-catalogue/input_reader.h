#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>


#include "transport_catalogue.h"

namespace input {

class Parser {
 public:

  void LoadInputQueries(std::istream& in, TransportCatalogue& catalogue);


 private:
  void ParseQuery(std::string_view query);
  void ParseStopQuery(std::string_view query, TransportCatalogue& catalogue);
  void ParseBusQuery(std::string_view query, TransportCatalogue& catalogue);
  void ParseAllQueries(TransportCatalogue& catalogue);
  void ParseCicleBusQuery(std::string_view query, TransportCatalogue& catalogue);
  void ParseDistances(TransportCatalogue& catalogue);

  std::deque<std::string> deque_queries;
  std::map<std::string_view, std::vector<std::pair<std::string_view, int>>> dist;
};

}

