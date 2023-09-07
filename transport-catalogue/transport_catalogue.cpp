#include "transport_catalogue.h"

void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view>& stops) {
  std::vector<Stop*> tmp;
  std::set<std::string_view> unique;
  for (auto stop : stops) {
    tmp.push_back(stop_name_to_stop.at(stop));
    unique.insert(stop);
    stop_to_buses[stop].insert( (name));
  }
  all_buses_.push_back({name, tmp, unique});
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
  size_t unique = bus.unique_stops.size();
  double length = 0;
  int real = 0;
  for (size_t i = 0; i < stops - 1; ++i) {
    length += ComputeDistance(bus.route[i]->coordinates, bus.route[i + 1]->coordinates);
    ParseStops(*bus.route[i]);
    ParseStops(*bus.route[i+1]);
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

void TransportCatalogue::ParseStops(const Stop& stop_)  {
  std::string_view stop_name = stop_.name;
  for(auto[stop, dist]:stop_.stops) {
    distances_.insert({{stop_name_to_stop.at(stop_name), stop_name_to_stop.at(stop)},  dist});
  }

}
int TransportCatalogue::ComputeRealDist(Stop* stop1, Stop* stop2) const{
  if (!distances_.count({stop1, stop2})) {
    return distances_.at({stop2, stop1});
  }
  return distances_.at({stop1, stop2});
}
