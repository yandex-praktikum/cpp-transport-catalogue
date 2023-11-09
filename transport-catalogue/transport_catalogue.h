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


  void PutRoutingInfo(int bus_velocity, int wait_time) {
    velocity_ = bus_velocity;
    wait_time_ = wait_time;
  }

  graph::Router<double> ProcessGraph() ;


void PrintRoute(const std::optional<graph::Router<double>::RouteInfo>& info) const {
  if (!info.has_value()) {
    std::cout << "No WAY!\n";
    return;
  }
  bool flag = false;
  for (auto item: info->edges) {
    auto edges = graph_.GetEdge(item);
    if (!flag) {
      std::cout << id_to_stop.at(edges.from) << " " << id_to_stop.at(edges.to) << " " << edges.stop_counter << " " << edges.weight << " " ;
    }
    else {
      std::cout << id_to_stop.at(edges.to) << " " <<edges.stop_counter << " " << edges.weight << ' ';
    }
    flag = true;

  }
  std::cout << "\n" << info->weight << "\n";

}

std::optional<std::pair<size_t, size_t>> GetIds (std::string_view stop1, std::string_view stop2) {
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

graph::Edge<double> GetEdgeFromCatalogue(size_t id) const {
  return graph_.GetEdge(id);
}
std::string_view GetStringFromId(size_t id) const {
  return id_to_stop.at(id);
}

int GetWaitingTime() const {
  return wait_time_;
}


 private:


  std::list<Stop> all_stops_;
  std::unordered_map<std::string_view , Stop*> stop_name_to_stop;

  std::list<Bus> all_buses_;
  std::unordered_map<std::string_view , Bus*> bus_name_to_bus;
  std::unordered_map<std::string_view , std::set<std::string_view>> stop_to_buses;
  std::unordered_map<PairStop, int, Hasher> distances_;

  graph::DirectedWeightedGraph<double> graph_;
  int velocity_ = 0, wait_time_ = 0;

  std::unordered_map<std::string_view, size_t> stop_to_id;
  std::unordered_map<size_t , std::string_view> id_to_stop;




  int ComputeRealDist(Stop* stop1, Stop* stop2) const;

  size_t UniqueStops(const std::vector<Stop*>& stops);

  void ProcessCicledRout(const std::vector<Stop*>& stops, std::string_view bus_name);

  void ProcessNonCycleRout(const std::vector<Stop*>& stops, std::string_view bus_name);



};
