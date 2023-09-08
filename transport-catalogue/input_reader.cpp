#include "input_reader.h"
namespace input {

void Parser::LoadInputQueries(std::istream& in, TransportCatalogue& catalogue) {
  int buses_quantity;
  in >> buses_quantity;
  in.ignore();
  std::string query;
  for (int i = 0; i < buses_quantity; ++i) {
    std::getline(in, query);
    ParseQuery(query);
  }
  ParseAllQueries(catalogue);
  ParseDistances(catalogue);
}
void Parser::ParseQuery(std::string_view query) {
  if (query[0] == 'S') {
    deque_queries.emplace_front(query);
  } else {
    deque_queries.emplace_back(query);
  }
}

void Parser::ParseBusQuery(std::string_view query, TransportCatalogue& catalogue) {
  std::vector<std::string_view> stops;
  std::string_view name;

  size_t colonPos = query.find(':');
  // Извлечение названия автобуса
  name = query.substr(4, colonPos - 4); // Пропускаем "Bus " в начале строки

  // Извлечение остановок
  std::string_view stopsStr = query.substr(colonPos + 2); // Пропуск двоеточия и пробела
  size_t startPos = 0;
  size_t stopPos = stopsStr.find(" > ", startPos);
  while (stopPos != std::string_view::npos) {
    stops.push_back(stopsStr.substr(startPos, stopPos - startPos));
    startPos = stopPos + 3; // Пропуск " > "
    stopPos = stopsStr.find(" > ", startPos);
  }
  stops.push_back(stopsStr.substr(startPos));
  catalogue.AddBus(name, stops);
}

void Parser::ParseStopQuery(std::string_view query, TransportCatalogue& catalogue) {

  size_t pos = 4;

  // Читаем название строки
  pos = query.find_first_not_of(' ', pos);
  size_t colonPos = query.find(':', pos);
  std::string_view stopName = query.substr(pos, colonPos - pos);
  pos = colonPos + 1;

  // Читаем два числа типа double
  pos = query.find_first_not_of(' ', pos);
  size_t commaPos = query.find(',', pos);

  double lat = std::stod(std::string(query.substr(pos, commaPos - pos)));
  pos = commaPos + 1;

  pos = query.find_first_not_of(' ', pos);
  commaPos = query.find(',', pos);

  double lon = std::stod(std::string(query.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  if (commaPos == std::string_view::npos) {
    catalogue.AddStop({stopName, {lat, lon}});
    return;
  }
  // Читаем остановки и расстояния
  std::vector<std::pair<std::string_view, int>> stops;
  while (pos < query.size()) {
    pos = query.find_first_not_of(' ', pos + 1);
    commaPos = query.find(',', pos + 1);
    if (commaPos == std::string::npos) {
      commaPos = query.size();
    }
    std::string_view distance_str = (query.substr(pos, commaPos - pos));
    std::string_view distance = distance_str.substr(0, distance_str.find(('m')));
    int dis = std::stoi(std::string(distance));
    pos = query.find("to", pos);
    commaPos = query.find(',', pos);
    if (commaPos == std::string::npos) {
      commaPos = query.size();
    }
    pos += 3;
    std::string_view stop_name = query.substr(pos, commaPos - pos);
    pos = commaPos;
    dist[stopName].emplace_back(stop_name, dis);
  }
  catalogue.AddStop({stopName, {lat, lon}});

}
void Parser::ParseCicleBusQuery(std::string_view query, TransportCatalogue& catalogue) {
  std::vector<std::string_view> stops;
  std::string_view name;

  size_t colonPos = query.find(':');

  // Извлечение названия автобуса
  name = query.substr(4, colonPos - 4); // Пропускаем "Bus " в начале строки

  // Извлечение остановок
  std::string_view stopsStr = query.substr(colonPos + 2); // Пропуск двоеточия и пробела
  size_t startPos = 0;
  size_t stopPos = stopsStr.find(" - ", startPos);
  while (stopPos != std::string_view::npos) {
    stops.push_back(stopsStr.substr(startPos, stopPos - startPos));
    startPos = stopPos + 3; // Пропуск " - "
    stopPos = stopsStr.find(" - ", startPos);
  }
  stops.push_back(stopsStr.substr(startPos));
  for (int64_t i = static_cast<int64_t>(stops.size()) - 2; i != -1; --i) {
    stops.push_back(stops[i]);
  }
  catalogue.AddBus(name, stops);
}

void Parser::ParseAllQueries(TransportCatalogue& catalogue) {
  for (auto& query : deque_queries) {
    if (query[0] == 'S') {
      ParseStopQuery(query, catalogue);
    } else {
      if (query.find(" > ") != std::string::npos) {
        ParseBusQuery(query, catalogue);
      } else {
        ParseCicleBusQuery(query, catalogue);

      }
    }
  }
}
void Parser::ParseDistances(TransportCatalogue& catalogue) {
  for (auto& [stop_name1, stops] : dist) {
    for(auto [other_stop_name, distance]: stops) {
      catalogue.AddDistanceInfo(stop_name1, other_stop_name, distance);
    }
  }
}
}
