#include "stat_reader.h"
namespace output {
void WriteBusInFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue) {
  std::string name = query.substr(4, query.size());
  auto bus = catalogue.FindBus(name);
  if (bus == nullptr) {
    file << query << ": not found\n";
    return;
  }
  auto info = catalogue.GetBusInfo(*bus);
  file << query << ": " << info.stops_on_route << " stops on route, " << info.unique_stops << " unique stops, "
       << info.real_lenght << " route length, " <<  std::fixed << std::setprecision(6) <<info.real_lenght / info.route_length << " curvature\n";

}
void WriteBus(const std::string& query, TransportCatalogue& catalogue) {
  std::string name = query.substr(4, query.size());
  auto bus = catalogue.FindBus(name);
  if (bus == nullptr) {
    std::cout << query << ": not found\n";
    return;
  }
  auto info = catalogue.GetBusInfo(*bus);
  std::cout << query << ": " << info.stops_on_route << " stops on route, " << info.unique_stops << " unique stops, "
       << info.real_lenght << " route length, " <<  std::fixed << std::setprecision(6) <<info.real_lenght / info.route_length << " curvature\n";
}

void WriteStopInFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue) {
  std::string name = query.substr(5, query.size());
  auto stop = catalogue.FindStop(name);
  if (stop == nullptr) {
    file << query << ": not found\n";
    return;
  }
  auto buses = catalogue.GetStopInfo(*stop);
  file << query << ": ";
  if (buses.empty()) {
    file << "no buses\n";
    return;
  }
  file << "buses ";
  bool flag = true;
  for (auto& bus : buses) {
    if (!flag) {
      file << " ";
    }
    file << bus;
    flag = false; // Устанавливаем флаг в false после первого элемента
  }
  file << "\n";

}
void WriteStop(const std::string& query, TransportCatalogue& catalogue) {
  std::string name = query.substr(5, query.size());
  auto stop = catalogue.FindStop(name);
  if (stop == nullptr) {
    std::cout << query << ": not found\n";
    return;
  }
  auto buses = catalogue.GetStopInfo(*stop);
  std::cout << query << ": ";
  if (buses.empty()) {
    std::cout << "no buses\n";
    return;

  }
  std::cout << "buses ";
  bool flag = true;
  for (auto bus : buses) {
    if (!flag) {
      std::cout << " ";
    }
    std::cout << bus;
    flag = false; // Устанавливаем флаг в false после первого элемента
  }
  std::cout << "\n";
}
void ParseQueryFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue) {
  if(query[0] == 'B') {
    WriteBusInFile(file, query, catalogue);
  }
  else {
    WriteStopInFile(file,query, catalogue);
  }

}

void ParseQuery(const std::string& query, TransportCatalogue& catalogue) {
  if(query[0] == 'B') {
    WriteBus(query, catalogue);
  }
  else {
    WriteStop(query, catalogue);
  }
}

}
