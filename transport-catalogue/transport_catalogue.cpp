#include "transport_catalogue.h"
#include <iostream>

namespace transport_db
{
    using namespace domain;

    void TransportCatalogue::AddStop(const Stop &stop)
    {
        stops_.push_back(stop);
        std::string_view stop_name(stops_.back().name);
        stop_index_[stop_name] = &stops_.back();
    }

    void TransportCatalogue::AddSegment(const std::string &stop, const std::string &next_stop, int distance)
    {
        Stop *stop_ptr = stop_index_.at(stop);
        Stop *next_stop_ptr = stop_index_.at(next_stop);
        distances_[{stop_ptr, next_stop_ptr}] = distance;
    }

    void TransportCatalogue::AddRoute(const std::string &name, const std::vector<std::string> &stops, const RouteType &type)
    {
        Bus new_route;
        new_route.route_number = std::move(name);
        new_route.type = std::move(type);
        routes_.push_back(std::move(new_route));
        Bus *bus_ptr = &routes_.back();
        std::string_view route_name(bus_ptr->route_number);
        for (auto &stop : stops)
        {
            Stop *stop_ptr = stop_index_.at(stop);
            (bus_ptr->stops).push_back(stop_ptr);
            stop_ptr->route_numbers.insert(route_name);
        }
        route_index_[route_name] = &routes_.back();
    }

    bool TransportCatalogue::FindStop(std::string_view stop_name) const
    {
        return stop_index_.count(stop_name);
    }
    const Stop *TransportCatalogue::StopInfo(std::string_view stop_name) const
    {
        return stop_index_.at(stop_name);
    }

    bool TransportCatalogue::FindRote(std::string_view route_number) const
    {
        return route_index_.count(route_number);
    }

    bool TransportCatalogue::FindSegment(const Segment &segment) const
    {
        return  distances_.count(segment);
    }

    const Bus *TransportCatalogue::RouteInfo(std::string_view route_number) const
    {
        return route_index_.at(route_number);
    }

    int TransportCatalogue::SegmentInfo(const Segment &segment) const
    {
        return distances_.at(segment);
    }
}