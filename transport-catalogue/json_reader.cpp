#include "json_reader.h"

namespace json_input {
void Parser::LoadInputQueries(const json::Array& list, TransportCatalogue& catalogue) {
  for (const auto& one_query : list) {
    ParseQuery(one_query.AsDict());
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
    auto tmp_query = query.AsDict();
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
  for (int64_t i = static_cast<int64_t>(bus_stops.size()) - 2; i > -1; --i) {
    bus_stops.push_back(bus_stops[i]);
  }
  catalogue.AddBus(name, bus_stops, false);
}

void Parser::ParseStopQuery(const json::Dict& stop, TransportCatalogue& catalogue) {
  all_queries.push_back(stop.at("name").AsString());
  std::string_view name = all_queries.back();
  double latitude = stop.at("latitude").AsDouble();
  double longitude = stop.at("longitude").AsDouble();
  auto distances = stop.at("road_distances").AsDict();
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

void Parser::LoadRoutQuery(const json::Dict& dict, TransportCatalogue& catalogue) {
  auto velocity = dict.at("bus_velocity").AsInt();
  auto wait_time = dict.at("bus_wait_time").AsInt();
  catalogue.PutRoutingInfo(velocity, wait_time);
}

} // namespace json_input

namespace json_output {
void WriteBus(const json::Dict& bus, TransportCatalogue& catalogue, json::Builder& ans) {
  auto tmp_bus = catalogue.FindBus(bus.at("name").AsString());
  int id = bus.at("id").AsInt();
  if (tmp_bus == nullptr) {
    ans.StartDict().Key("error_message").Value("not found").Key("request_id").Value(id).EndDict();
    //ans.emplace_back(json::Dict{{"error_message", "not found"}, {"request_id", id}});
    return;
  }

  auto info = catalogue.GetBusInfo(*tmp_bus);
  ans.StartDict().Key("curvature").Value(info.real_lenght/info.route_length).Key("route_length").Value(info.real_lenght).Key("unique_stop_count").Value(static_cast<int>(info.unique_stops));
  ans.Key("stop_count").Value(static_cast<int>(info.stops_on_route)).Key("request_id").Value(id).EndDict();
 /* ans.emplace_back(json::Dict{
      {"curvature", info.real_lenght/info.route_length},
      {"route_length", info.real_lenght},
      {"unique_stop_count", static_cast<int>(info.unique_stops)},
      {"stop_count", static_cast<int>(info.stops_on_route)}, {"request_id",id }
  });*/
}

void WriteStop(const json::Dict& stop, TransportCatalogue& catalogue, json::Builder& ans) {
  auto tmp_stop = catalogue.FindStop(stop.at("name").AsString());
  int id = stop.at("id").AsInt();
  if (tmp_stop == nullptr) {
    ans.StartDict().Key("error_message").Value("not found").Key("request_id").Value(id).EndDict();
   // ans.emplace_back(json::Dict{{"error_message", "not found"}, {"request_id", id}});
    return;
  }
  auto buses = catalogue.GetStopInfo(*tmp_stop);
  json::Array tmp;
  for(auto i : buses) {
    tmp.emplace_back(std::string(i));
  }
  ans.StartDict().Key("buses").Value(tmp).Key("request_id").Value(id).EndDict();
  //ans.emplace_back(json::Dict{{"buses", tmp}, {"request_id", id}});
}

void WriteMap(const json::Dict& stop, TransportCatalogue& catalogue, json::Builder& ans, const json::Dict& settings) {
  int id = stop.at("id").AsInt();
  std::ostringstream out;
  render::LoadRenderInformation(out, catalogue, settings);
  ans.StartDict().Key("map").Value(out.str()).Key("request_id").Value(id).EndDict();
  //ans.emplace_back(json::Dict{{"map", out.str()} ,{"request_id", id}});

}
void WriteRoute(const json::Dict& rout, json::Builder& ans, TransportRouter& router) {
  auto from  = rout.at("from").AsString();
  int id = rout.at("id").AsInt();
  auto to = rout.at("to").AsString();
  auto info = router.FindRoute(from, to);
  if (info.count("error_message")) {
    ans.StartDict().Key("error_message").Value(info["error_message"].AsString()).Key("request_id").Value(id).EndDict();
    return;
  }

  ans.StartDict().Key("items").Value(info["items"].AsArray()).Key("total_time").Value(info["total_time"].AsDouble()).Key("request_id").Value(id).EndDict();
}

json::Node LoadOutputQueries(const json::Array& list, TransportCatalogue& catalogue, const json::Dict& settings, TransportRouter& router) {
  json::Builder ans;
  ans.StartArray();
  for (const auto& output : list) {
    auto tmp_dict = output.AsDict();
    if (tmp_dict.at("type").AsString() == "Bus") {
      WriteBus(tmp_dict, catalogue, ans);
    } else if (tmp_dict.at("type").AsString() == "Map") {
      WriteMap(tmp_dict, catalogue, ans, settings);
    } else if (tmp_dict.at("type").AsString() == "Stop") {
      WriteStop(tmp_dict, catalogue, ans);
    } else if (tmp_dict.at("type").AsString() == "Route") {
      WriteRoute(tmp_dict,  ans, router);
    }
  }
  ans.EndArray();
  return ans.Build();
}






}

// namespace json_output
