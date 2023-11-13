#include "transport_router.h"

void TransportRouter::ProcessCicledRout(const std::vector<TransportCatalogue::Stop*>& stops,
                                        std::string_view bus_name) {
  size_t id = id_to_stop.size();

  for (int i = 0; i < int(stops.size()) - 1; ++i) {
    auto stop_name = stops[i]->name;
    if (!stop_to_id.count(stop_name)) {
      stop_to_id[stops[i]->name] = id;
      id_to_stop[id++] = stops[i]->name;
    }
    double total_dist_for_i_bus = 0;
    auto stop1 = catalogue_.GetStopFromString(stops[i]->name);
    auto from = stop_to_id.at(stop1->name);
    for (int j = i + 1; j < stops.size(); ++j) {

      if (!stop_to_id.count(stops[j]->name)) {
        stop_to_id[stops[j]->name] = id;
        id_to_stop[id++] = stops[j]->name;
      }

      auto stop2 = catalogue_.GetStopFromString(stops[j]->name);

      auto to = stop_to_id.at(stop2->name);
      total_dist_for_i_bus += catalogue_.ComputeRealDist(stop1, stop2);
      graph_->AddEdge({from, to, ((total_dist_for_i_bus / 1000) / velocity_) * 60 + wait_time_, bus_name, j - i});
      stop1 = stop2;
    }
  }
}

void TransportRouter::ProcessNonCycleRout(const std::vector<TransportCatalogue::Stop*>& stops,
                                          std::string_view bus_name) {
  long n = long(stops.size());
  auto med = (n / 2) + 1;
  std::vector<TransportCatalogue::Stop*> tmp;
  for (int i = 0; i < med; ++i) {
    tmp.push_back(stops[i]);
  }
  ProcessCicledRout(tmp, bus_name);
  tmp.clear();
  for (int i = med - 1; i < n; ++i) {
    tmp.push_back(stops[i]);
  }
  ProcessCicledRout(tmp, bus_name);
}
std::optional<std::pair<size_t, size_t>> TransportRouter::GetIds(std::string_view stop1, std::string_view stop2) {
  std::optional<std::pair<size_t , size_t >> ans;
  if (!stop_to_id.count(stop1)) {
    return ans;
  }
  if (!stop_to_id.count(stop2)) {
    return ans;
  }
  ans = {stop_to_id.at(stop1), stop_to_id.at(stop2)};
  return ans;
}

void TransportRouter::ProcessGraph() {
  const auto& all_buses_ = catalogue_.GetAllBusesForRoute();
  for (const auto& bus : all_buses_) {
    if (bus.is_roundtrip) {
      ProcessCicledRout(bus.route, bus.name);
    } else {
      ProcessNonCycleRout(bus.route, bus.name);
    }
  }
  router.emplace(graph::Router(graph_.value()));
}

json::Dict TransportRouter::FindRoute(std::string_view from, std::string_view to) {
  auto cord = GetIds(from, to);
  if (!cord.has_value()) {
    return json::Dict{{"error_message", "not found"}};
  }
  auto [id_from, id_to] = cord.value();
  auto info = router->BuildRoute(id_from, id_to);
  if (!info.has_value()) {
    return json::Dict{{"error_message", "not found"}};
  }
  json::Array items;
  // auto tmp_edge = catalogue_.GetEdgeFromCatalogue(info->edges[0]);
  auto time = wait_time_;

  for (auto edge : info->edges) {
    items.emplace_back(json::Dict());
    auto tmp_edge = graph_->GetEdge(edge);
    auto stop_from = std::string(id_to_stop.at(tmp_edge.from));

    items.back() = json::Dict{{"stop_name", stop_from}, {"time", time}, {"type", "Wait"}};
    auto name = std::string(tmp_edge.bus_name);
    items.emplace_back(json::Dict());

    items.back() = json::Dict{{"bus", name}, {"span_count", tmp_edge.stop_counter}, {"time", tmp_edge.weight - time},
                              {"type", "Bus"}};

  }
  return json::Dict{{"total_time", info->weight}, {"items", items}};
}
void TransportRouter::PutRoutingInfo(int bus_velocity, int wait_time) {
  velocity_ = bus_velocity;
  wait_time_ = wait_time;
}
