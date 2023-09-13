#include "json_reader.h"

namespace json_input {
void Parser::LoadInputQueries(const json::Array& list, TransportCatalogue& catalogue) {
  for (const auto& one_query : list) {
    ParseQuery(one_query.AsMap());
  }
  ParseAllQueries(catalogue);
  ParseDistances(catalogue);
}
void Parser::ParseQuery(const json::Dict& query) {
  std::string type = "type";
  if (query.at(type) == "Bus") {
    deque_queries.emplace_back(query);
  } else {
    deque_queries.emplace_front(query);
  }
}

void Parser::ParseAllQueries(TransportCatalogue& catalogue) {
  for (const auto& query : deque_queries) {
    auto tmp_query = query.AsMap();
    if (tmp_query.at("type").AsString() == "Bus") {
      if (tmp_query.at("is_roundtrip").AsBool()) {
        ParseBusQuery(tmp_query, catalogue);
      } else {
        ParseCicleBusQuery(tmp_query, catalogue);
      }
    } else {
      ParseStopQuery(tmp_query, catalogue);

    }
  }
}
void Parser::ParseBusQuery(const json::Dict& bus, TransportCatalogue& catalogue) {
  all_queries.push_back(bus.at("name").AsString());
  std::string_view name = all_queries.back();
  auto stops = bus.at("stops").AsArray();
  std::vector<std::string_view> bus_stops;
  for (const auto& stop : stops) {
    all_queries.push_back(stop.AsString());
    bus_stops.push_back(all_queries.back());
  }
  catalogue.AddBus(name, bus_stops, true);
}

void Parser::ParseCicleBusQuery(const json::Dict& bus, TransportCatalogue& catalogue) {
  all_queries.push_back(bus.at("name").AsString());
  std::string_view name = all_queries.back();
  auto stops = bus.at("stops").AsArray();
  std::vector<std::string_view> bus_stops;
  for (const auto& stop : stops) {
    all_queries.push_back(stop.AsString());
    bus_stops.push_back(all_queries.back());
  }
  for (int64_t i = static_cast<int64_t>(bus_stops.size()) - 2; i != -1; --i) {
    bus_stops.push_back(bus_stops[i]);
  }
  catalogue.AddBus(name, bus_stops, false);
}

void Parser::ParseStopQuery(const json::Dict& stop, TransportCatalogue& catalogue) {
  all_queries.push_back(stop.at("name").AsString());
  std::string_view name = all_queries.back();
  double latitude = stop.at("latitude").AsDouble();
  double longitude = stop.at("longitude").AsDouble();
  auto distances = stop.at("road_distances").AsMap();
  for (const auto& [kStop, dist] : distances) {
    all_queries.push_back(kStop);
    dist_[name].emplace_back(all_queries.back(), dist.AsInt());
  }
  catalogue.AddStop({name, {latitude, longitude}});
}

void Parser::ParseDistances(TransportCatalogue& catalogue) {
  for (auto& [stop_name1, stops] : dist_) {
    for (auto [other_stop_name, distance] : stops) {
      catalogue.AddDistanceInfo(stop_name1, other_stop_name, distance);
    }
  }
}

} // namespace json_input

namespace json_output {
void WriteBus(const json::Dict& bus, TransportCatalogue& catalogue, json::Array& ans) {
  auto tmp_bus = catalogue.FindBus(bus.at("name").AsString());
  int id = bus.at("id").AsInt();
  if (tmp_bus == nullptr) {
    ans.emplace_back(json::Dict{{"error_message", "not found"}, {"request_id", id}});
    return;
  }

  auto info = catalogue.GetBusInfo(*tmp_bus);
  ans.emplace_back(json::Dict{
      {"curvature", info.real_lenght/info.route_length},
      {"route_length", info.real_lenght},
      {"unique_stop_count", static_cast<int>(info.unique_stops)},
      {"stop_count", static_cast<int>(info.stops_on_route)}, {"request_id",id }
  });
}

void WriteStop(const json::Dict& stop, TransportCatalogue& catalogue, json::Array& ans) {
  auto tmp_stop = catalogue.FindStop(stop.at("name").AsString());
  int id = stop.at("id").AsInt();
  if (tmp_stop == nullptr) {
    ans.emplace_back(json::Dict{{"error_message", "not found"}, {"request_id", id}});
    return;
  }
  auto buses = catalogue.GetStopInfo(*tmp_stop);
  json::Array tmp;
  for(auto i : buses) {
    tmp.emplace_back(std::string(i));
  }
  ans.emplace_back(json::Dict{{"buses", tmp}, {"request_id", id}});
}

void WriteMap(const json::Dict& stop, TransportCatalogue& catalogue, json::Array& ans, const json::Dict& settings) {
  int id = stop.at("id").AsInt();
  std::ostringstream out;
  render::LoadRenderInformation(out, catalogue, settings);
  ans.emplace_back(json::Dict{{"map", out.str()} ,{"request_id", id}});

}

json::Array LoadOutputQueries(const json::Array& list, TransportCatalogue& catalogue, const json::Dict& settings) {
  json::Array ans;
  for (const auto& output : list) {
    auto tmp_dict = output.AsMap();
    if (tmp_dict.at("type").AsString() == "Bus") {
      WriteBus(tmp_dict, catalogue, ans);
    } else if (tmp_dict.at("type").AsString() == "Map") {
      WriteMap(tmp_dict, catalogue, ans, settings);
    } else {
      WriteStop(tmp_dict, catalogue, ans);
    }
  }
  return ans;
}



}

// namespace json_output