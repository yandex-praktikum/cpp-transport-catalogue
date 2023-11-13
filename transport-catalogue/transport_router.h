#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json.h"





class TransportRouter {
 public:
  explicit TransportRouter(TransportCatalogue& catalogue) :catalogue_(catalogue) {
    graph_.emplace(graph::DirectedWeightedGraph<double>(catalogue.GetAllStopsSize()));
  }

  json::Dict FindRoute(std::string_view from, std::string_view to);
    void PutRoutingInfo(int bus_velocity, int wait_time);

  void ProcessCicledRout(const std::vector<TransportCatalogue::Stop*>& stops, std::string_view bus_name);

  void ProcessNonCycleRout(const std::vector<TransportCatalogue::Stop*>& stops, std::string_view bus_name);


  void ProcessGraph();

  std::optional<std::pair<size_t, size_t>> GetIds (std::string_view stop1, std::string_view stop2);

 private:
  TransportCatalogue& catalogue_;
  std::optional<graph::Router<double>> router;

  std::unordered_map<std::string_view, size_t> stop_to_id ;
  std::unordered_map<size_t , std::string_view> id_to_stop;


  int velocity_ = 0, wait_time_ = 0;
  std::optional<graph::DirectedWeightedGraph<double>> graph_;
};
