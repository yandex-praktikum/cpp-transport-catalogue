#pragma once
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <cstdint>
#include "geo.h"

namespace transport_db
{
    enum class RouteType
    {
        CIRCLE,
        LINEAR
    };

    class Stop
    {
    public:
        std::string name;
        Coordinates coord;
        std::set<std::string_view> route_numbers;
        size_t Hash() const;

    private:
        std::hash<int64_t> hasher_;
    };

    using Segment = std::pair<Stop *, Stop *>;

    class StopHasher
    {
    public:
        size_t operator()(const Segment &segment) const;
    };

    struct Bus
    {
        std::string route_number;
        std::vector<Stop *> stops;
        RouteType type;
    };

    class TransportCatalogue
    {

    public:
        TransportCatalogue() = default;

        void AddStop(const Stop &stop);
        void AddSegment(const std::string &stop, const std::string &next_stop, int distance);
        void AddRoute(const std::string &name, const std::vector<std::string> &stops, const RouteType &type);

        bool FindStop(std::string_view stop_name) const;
        bool FindRote(std::string_view route_number) const;
        bool FindSegment(const Segment &segment);

        const Stop *StopInfo(std::string_view stop_name) const;
        const Bus *RouteInfo(std::string_view route_number) const;
        int SegmentInfo(const Segment &segment) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> routes_;

        std::unordered_map<std::string_view, Stop *> stop_index_;
        std::unordered_map<std::string_view, Bus *> route_index_;

        std::unordered_map<Segment, int, StopHasher> distances_;
    };

}