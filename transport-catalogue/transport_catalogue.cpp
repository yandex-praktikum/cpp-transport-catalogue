#include "transport_catalogue.h"

void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view>& stops) {
  std::vector<Stop*> tmp;
  for (auto stop : stops) {
    tmp.push_back(stop_name_to_stop.at(stop));
    stop_to_buses[stop].insert( (name));
  }
  all_buses_.push_back({name, tmp});
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

TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const Bus& bus)  {
  size_t stops = bus.route.size();
  size_t unique = UniqueStops(bus.route);
  double length = 0;
  int real = 0;
  for (size_t i = 0; i < stops - 1; ++i) {
    length += ComputeDistance(bus.route[i]->coordinates, bus.route[i + 1]->coordinates);
    real+= ComputeRealDist(stop_name_to_stop.at(bus.route[i]->name), stop_name_to_stop.at(bus.route[i+1]->name));
  }
  return TransportCatalogue::BusInfo{stops, unique, real,length};
}
std::set<std::string_view> TransportCatalogue::GetStopInfo(const Stop& stop) {
  if(stop_to_buses.count((stop.name))) {
    return stop_to_buses.at((stop.name));
  }
  static std::set<std::string_view> empty;
  return empty;
  }


int TransportCatalogue::ComputeRealDist(Stop* stop1, Stop* stop2) const{
  if (!distances_.count({stop1, stop2})) {
    return distances_.at({stop2, stop1});
  }
  return distances_.at({stop1, stop2});
}

size_t TransportCatalogue::UniqueStops(const std::vector<Stop*>& stops) {
  std::vector<Stop*> uniq;
  uniq.reserve(stops.size());
  for(auto stop: stops) {
    uniq.push_back(stop);
  }
  std::sort(uniq.begin(), uniq.end());
  return std::unique(uniq.begin(), uniq.end()) - uniq.begin();
}

void TransportCatalogue::AddDistanceInfo(std::string_view stop1, std::string_view stop2, int dist) {
  distances_[{stop_name_to_stop.at(stop1), stop_name_to_stop.at(stop2)}] = dist;
}

