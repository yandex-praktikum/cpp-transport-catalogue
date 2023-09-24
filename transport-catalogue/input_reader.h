#pragma once
#include <iostream>
#include <string>
#include "transport_catalogue.h"
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <execution>
#include "geo.h"

namespace data_input
{
    struct StopInfo
    {
        StopInfo(transport_db::Stop &&stop, std::unordered_map<std::string, int> &&distances);
        transport_db::Stop stop;
        std::unordered_map<std::string, int> distances;
    };

    struct RouteInfo
    {
        RouteInfo(std::string &&name, std::vector<std::string> stops, transport_db::RouteType &&type);
        std::string name;
        std::vector<std::string> stops;
        transport_db::RouteType type;
    };
    namespace detail
    {
        std::pair<std::string_view, std::string_view> Split(std::string_view in, char delimeter);

        std::vector<std::string_view> MultiSplit(std::string_view in, char delimeter);

        double CoordDecode(std::string_view coords);
    }
    namespace process_request
    {
        StopInfo StopRequest(std::string_view line);

        RouteInfo RouteRequest(std::string_view line);
    }

    template <typename InStream>
    void ReadData(InStream &in, transport_db::TransportCatalogue &tc)
    {
        std::vector<RouteInfo> raw_route_data;
        std::vector<StopInfo> raw_stop_data;
        std::map<std::string, int> distances;
        std::string data;
        int counter;
        (in >> counter).get();
        while (counter > 0)
        {
            std::getline(in, data);
            size_t pos = data.find(" ");
            std::string_view left = std::string_view(data).substr(0, pos);
            std::string_view right = std::string_view(data).substr(pos + 1);
            if (left == "Stop")
            {
                raw_stop_data.push_back(std::move(process_request::StopRequest(right)));
            }
            else
            {
                raw_route_data.push_back(std::move(process_request::RouteRequest(right)));
                ;
            }
            --counter;
        }
        std::for_each(std::execution::seq, raw_stop_data.begin(), raw_stop_data.end(),
                      [&](auto &data)
                      {
                          tc.AddStop(data.stop);
                      });
        std::for_each(std::execution::seq, raw_stop_data.begin(), raw_stop_data.end(),
                      [&](auto &stop_data)
                      {
                          for (const auto &[next_stop, distance] : stop_data.distances)
                          {
                              tc.AddSegment(stop_data.stop.name, next_stop, distance);
                          }
                      });
        std::for_each(std::execution::seq, raw_route_data.begin(), raw_route_data.end(),
                      [&](auto &route)
                      {
                          tc.AddRoute(route.name, route.stops, route.type);
                      });
    }
}