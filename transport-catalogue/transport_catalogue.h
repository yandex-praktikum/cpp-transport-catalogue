#pragma once
#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <variant>
#include <map>



#include "geo.h"
#include "graph.h"
#include "router.h"

class TransportCatalogue {
 public:
  struct Stop {
    std::string_view name;
    geo::Coordinates coordinates;
  };
  struct Bus {
    std::string_view name;
    std::vector<Stop*> route;
    bool is_roundtrip;
  };

  struct BusInfo {
    size_t stops_on_route;
    size_t unique_stops;
    int real_lenght;
    double route_length;
  };
  using PairStop = std::pair<Stop*, Stop*>;
  struct Hasher {
    size_t operator()( const PairStop& pair) const {
      return 37* std::hash<std::string_view>{}(pair.first->name) + std::hash<std::string_view>{}(pair.second->name);
    }
  };
  void AddStop(const Stop& stop);
  Stop* FindStop(std::string_view query) const;

  void AddBus(std::string_view name, const std::vector<std::string_view>& stops, bool is_round);

  Bus* FindBus(std::string_view query) const;

  BusInfo GetBusInfo(const Bus& bus);
  std::set<std::string_view> GetStopInfo(const Stop& stop);

  void AddDistanceInfo(std::string_view stop1, std::string_view stop2, int dist);


  std::set<std::string_view > GetAllBuses() const;



size_t GetAllStopsSize() const{
  return all_stops_.size();
}
const std::list<Bus>& GetAllBusesForRoute() const {
  return all_buses_;
}

 Stop* GetStopFromString(std::string_view stop_name) const {
  return stop_name_to_stop.at(stop_name);
}
 int ComputeRealDist( Stop* stop1,  Stop* stop2) const;
 private:


  std::list<Stop> all_stops_;
  std::unordered_map<std::string_view , Stop*> stop_name_to_stop;

  std::list<Bus> all_buses_;
  std::unordered_map<std::string_view , Bus*> bus_name_to_bus;
  std::unordered_map<std::string_view , std::set<std::string_view>> stop_to_buses;
  std::unordered_map<PairStop, int, Hasher> distances_;



  size_t UniqueStops(const std::vector<Stop*>& stops);




};
