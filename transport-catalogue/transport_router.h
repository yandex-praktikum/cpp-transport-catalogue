#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json.h"





class TransportRouter {
 public:
  explicit TransportRouter(TransportCatalogue& catalogue) :catalogue_(catalogue) {
    graph_ = (graph::DirectedWeightedGraph<double>(catalogue.GetAllStopsSize()));
  }
// метод возвращает json::Dict, так как содержит данные разных типов, которые можно собрать за один проход по маршруту
// разделение его на несколько других методов вызовет ненужное увеличение объема кода, повисит сложность восприятия и дополнительную нагрузку из-за каста данных
// здесь удобнее использовать готовую структуру, которая позволяет правильно хранить данные разных типов

  json::Dict FindRoute(std::string_view from, std::string_view to);

  void SetRoutingInfo(int bus_velocity, int wait_time);

  void ProcessGraph(); // метод вызывается в main()


 private:


  std::optional<std::pair<size_t, size_t>> GetIds (std::string_view stop1, std::string_view stop2);


  void ProcessCicledRout(const std::vector<TransportCatalogue::Stop*>& stops, std::string_view bus_name);

  void ProcessNonCycleRout(const std::vector<TransportCatalogue::Stop*>& stops, std::string_view bus_name);

  TransportCatalogue& catalogue_;
  std::optional<graph::Router<double>> router;

  std::unordered_map<std::string_view, size_t> stop_to_id ;
  std::unordered_map<size_t , std::string_view> id_to_stop;


  int velocity_ = 0, wait_time_ = 0;
  std::optional<graph::DirectedWeightedGraph<double>> graph_;
};
