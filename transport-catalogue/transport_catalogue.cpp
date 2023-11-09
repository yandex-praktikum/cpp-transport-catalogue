#include "transport_catalogue.h"

void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view>& stops, bool is_round) {
  std::vector<Stop*> tmp;
  for (auto stop : stops) {
    tmp.push_back(stop_name_to_stop.at(stop));
    stop_to_buses[stop].insert((name));
  }

  all_buses_.push_back({name, tmp, is_round});
  bus_name_to_bus[name] = &all_buses_.back();
}
void TransportCatalogue::AddStop(const Stop& stop) {
  all_stops_.push_back(stop);
  stop_name_to_stop[stop.name] = &all_stops_.back();
}
TransportCatalogue::Bus* TransportCatalogue::FindBus(std::string_view query) const {
  if (!bus_name_to_bus.count(query)) {
    return nullptr;
  }
  return bus_name_to_bus.at(query);
}
TransportCatalogue::Stop* TransportCatalogue::FindStop(std::string_view query) const {
  if (!stop_name_to_stop.count(query)) {
    return nullptr;
  }
  return stop_name_to_stop.at(query);
}

TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const Bus& bus) {
  size_t stops = bus.route.size();
  size_t unique = UniqueStops(bus.route);
  double length = 0;
  int real = 0;
  for (size_t i = 0; i < stops - 1; ++i) {
    length += ComputeDistance(bus.route[i]->coordinates, bus.route[i + 1]->coordinates);
    real += ComputeRealDist(stop_name_to_stop.at(bus.route[i]->name), stop_name_to_stop.at(bus.route[i + 1]->name));
  }
  return TransportCatalogue::BusInfo{stops, unique, real, length};
}
std::set<std::string_view> TransportCatalogue::GetStopInfo(const Stop& stop) {
  if (stop_to_buses.count((stop.name))) {
    return stop_to_buses.at((stop.name));
  }
  static std::set<std::string_view> empty;
  return empty;
}

int TransportCatalogue::ComputeRealDist(Stop* stop1, Stop* stop2) const {
  if (!distances_.count({stop1, stop2})) {
    return distances_.at({stop2, stop1});
  }
  return distances_.at({stop1, stop2});
}

size_t TransportCatalogue::UniqueStops(const std::vector<Stop*>& stops) {
  std::vector<Stop*> uniq;
  uniq.reserve(stops.size());
  for (auto stop : stops) {
    uniq.push_back(stop);
  }
  std::sort(uniq.begin(), uniq.end());
  return std::unique(uniq.begin(), uniq.end()) - uniq.begin();
}

void TransportCatalogue::AddDistanceInfo(std::string_view stop1, std::string_view stop2, int dist) {
  if (!distances_.count({stop_name_to_stop.at(stop1), stop_name_to_stop.at(stop2)})) {
    distances_[{stop_name_to_stop.at(stop1), stop_name_to_stop.at(stop2)}] = dist;
  }
}
std::set<std::string_view> TransportCatalogue::GetAllBuses() const {
  std::set<std::string_view> ans;
  for (const auto& bus : all_buses_) {
    ans.insert(bus.name);
  }
  return ans;
}

graph::Router<double> TransportCatalogue::ProcessGraph() {
  graph_ = graph::DirectedWeightedGraph<double>(all_stops_.size());
  for (const auto& bus : all_buses_) {

    if (bus.is_roundtrip) {
      ProcessCicledRout(bus.route, bus.name);
    } else {
      ProcessNonCycleRout(bus.route, bus.name);
    }
  }

  return graph::Router(graph_);
}

void TransportCatalogue::ProcessCicledRout(const std::vector<Stop*>& stops, std::string_view bus_name) {
  size_t id = id_to_stop.size();
  std::set<std::pair<size_t, size_t >> unique_stops;
  for (int i = 0; i < int(stops.size()) - 1; ++i) {
    if (!stop_to_id.count(stops[i]->name)) {
      stop_to_id[stops[i]->name] = id;
      id_to_stop[id++] = stops[i]->name;
    }
    double total_dist_for_i_bus = 0;
    auto stop1 = stop_name_to_stop.at(stops[i]->name);
    auto from = stop_to_id.at(stop1->name);
    for (int j = i + 1; j < stops.size(); ++j) {

      if (!stop_to_id.count(stops[j]->name)) {
        stop_to_id[stops[j]->name] = id;
        id_to_stop[id++] = stops[j]->name;
      }


      auto stop2 = stop_name_to_stop.at(stops[j]->name);


      auto to = stop_to_id.at(stop2->name);
      unique_stops.insert({from, to});
      total_dist_for_i_bus += ComputeRealDist(stop1, stop2);
      graph_.AddEdge({from, to, ((total_dist_for_i_bus / 1000) / velocity_)* 60  + wait_time_ , bus_name,j - i});
      stop1 = stop2;
    }
  }
}

void TransportCatalogue::ProcessNonCycleRout(const std::vector<Stop*>& stops, std::string_view bus_name) {
  long n = long(stops.size());
  auto med = (n / 2) + 1;
  std::vector<Stop*> tmp;
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
