#include "stat_reader.h"
namespace output {

void WriteBus( std::ostream& out, const std::string& bus_info, TransportCatalogue& catalogue) {
  std::string name = bus_info.substr(4, bus_info.size());
  auto bus = catalogue.FindBus(name);
  if (bus == nullptr) {
    out << bus_info << ": not found\n";
    return;
  }
  auto info = catalogue.GetBusInfo(*bus);
  out << bus_info << ": " << info.stops_on_route << " stops on route, " << info.unique_stops << " unique stops, "
       << info.real_lenght << " route length, " <<  std::fixed << std::setprecision(6) <<info.real_lenght / info.route_length << " curvature\n";
}


void WriteStop(std::ostream& out, const std::string& query,  TransportCatalogue& catalogue) {
  std::string name = query.substr(5, query.size());
  auto stop = catalogue.FindStop(name);
  if (stop == nullptr) {
    out << query << ": not found\n";
    return;
  }
  auto buses = catalogue.GetStopInfo(*stop);
  out << query << ": ";
  if (buses.empty()) {
    out << "no buses\n";
    return;

  }
  out << "buses ";
  bool flag = true;
  for (auto bus : buses) {
    if (!flag) {
      out << " ";
    }
    out << bus;
    flag = false; // Устанавливаем флаг в false после первого элемента
  }
  out << "\n";
}


void ParseOutput(std::ostream& out, const std::string& query,  TransportCatalogue& catalogue) {
  if(query[0] == 'B') {
    WriteBus(out, query, catalogue);
  }
  else {
    WriteStop(out, query, catalogue);
  }
}

void LoadOutputQueries(std::istream& in, std::ostream& out, TransportCatalogue& catalogue) {
  int output_quantity;
  in >> output_quantity;
  in.ignore();
  std::string query;
  for(int i = 0; i < output_quantity; ++i) {
    std::getline(in, query);
    ParseOutput(out, query, catalogue);
  }
}

}
