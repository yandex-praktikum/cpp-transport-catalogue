#pragma once
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <cstdint>
#include "geo.h"
#include "domain.h"

namespace transport_db
{
    class TransportCatalogue
    {

    public:
        TransportCatalogue() = default;

        void AddStop(const domain::Stop &stop);
        void AddSegment(const std::string &stop, const std::string &next_stop, int distance);
        void AddRoute(const std::string &name, const std::vector<std::string> &stops, const domain::RouteType &type);

        bool FindStop(std::string_view stop_name) const;
        bool FindRote(std::string_view route_number) const;
        bool FindSegment(const domain::Segment &segment) const;

        const domain::Stop *StopInfo(std::string_view stop_name) const;
        const domain::Bus *RouteInfo(std::string_view route_number) const;
        int SegmentInfo(const domain::Segment &segment) const;

        std::set<std::string_view> GetAllRoutes() const;
      

    private:
        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> routes_;

        std::unordered_map<std::string_view, domain::Stop *> stop_index_;
        std::unordered_map<std::string_view, domain::Bus *> route_index_;

        std::unordered_map<domain::Segment, int, domain::StopHasher> distances_;
    };

}