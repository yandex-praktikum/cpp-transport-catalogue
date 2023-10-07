#pragma once
#include <iostream>
#include "transport_catalogue.h"
#include "geo.h"
#include <string>
#include <vector>

namespace data_output
{


    void ProcessTcRequests(std::istream &in, transport_db::TransportCatalogue &tc);

    void ProcessRequest(std::istream &in, transport_db::TransportCatalogue &tc);
    namespace statistics
    {
        void StopStat(std::string_view request, transport_db::TransportCatalogue &tc);

        void RouteStat(std::string_view request, transport_db::TransportCatalogue &tc);
        namespace detail
        {
            double StraightRouteLen(const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);

            double RealRouteLen(transport_db::TransportCatalogue &tc, const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);

            int CalcUnique(const std::vector<domain::Stop *> &stops);
        }
    }
    namespace output_methods
    {
        void PrintStopStat(std::string_view stop_name, const std::set<std::string_view> &routes);
        void StopNotFound(std::string_view requested_stop);

        void PrintRouteStat(std::string_view route, int stop_number, int unique_stops, double route_len, double curvature);
        void RouteNotFound(std::string_view requested_route);
    }
}