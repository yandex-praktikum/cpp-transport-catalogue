#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"


class TransportRouter {
 public:
  explicit TransportRouter(TransportCatalogue& catalogue) : catalogue_(catalogue){}

  json::Dict FindRoute(std::string_view from, std::string_view to) {
    auto cord = catalogue_.GetIds(from, to);
    if (!cord.has_value()) {
      return json::Dict {{"error_message", "not found"}};
    }
    auto [id_from, id_to] = cord.value();
    auto info = router->BuildRoute(id_from, id_to);
    if(!info.has_value()) {
      return json::Dict {{"error_message", "not found"}};
    }
    json::Array items;
   // auto tmp_edge = catalogue_.GetEdgeFromCatalogue(info->edges[0]);
   auto time = catalogue_.GetWaitingTime();

    for (auto edge: info->edges) {
      items.emplace_back(json::Dict());
      auto tmp_edge = catalogue_.GetEdgeFromCatalogue(edge);
      auto stop_from = std::string(catalogue_.GetStringFromId(tmp_edge.from));

      items.back() = json::Dict {{"stop_name", stop_from}, {"time", time}, {"type", "Wait"}};
      auto name = std::string (tmp_edge.bus_name);
      items.emplace_back(json::Dict());

      items.back() = json::Dict{{"bus", name}, {"span_count", tmp_edge.stop_counter}, {"time", tmp_edge.weight - time}, {"type", "Bus"}};

    }
    return json::Dict {{"total_time", info->weight}, {"items", items}};



  }

 private:
  TransportCatalogue& catalogue_;
  std::optional<graph::Router<double>> router = catalogue_.ProcessGraph();
};
