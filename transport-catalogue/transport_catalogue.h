#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "geo.h"

namespace TransportCatalogue {

    struct Stop{
        std::string name;
        double width;
        double longitude;
    };

    struct Router{
        std::string name;
        std::vector<Stop*> stops;
    };

    struct Bus{
        std::string name;
        std::vector<std::string> stops;
    };

    struct RouterInfo{
        RouterInfo(std::string name){
            this->name = name;
        }
        std::string name;
        size_t counterStops = 0;
        size_t counterUniqueStops = 0;
        int lenghtRouter = 0;
        double curvature = .0;
    };

    struct StopInfo{
        StopInfo(std::string name){
            this->name = name;
        }
        std::string name;
        std::set<std::string> buses;
        bool isFound = false;
    };

    class transport_catalogue
    {
    public:
        transport_catalogue();
        void load_stop(const Stop&);
        void load_router(const Bus&);
        RouterInfo getRouteInfo(const std::string& router) const;
        StopInfo getStopInfo(const std::string& stop);
        bool addDistance(std::string_view out,std::string_view to,const size_t distance);

    private:
        std::deque<Stop> _stops;
        std::unordered_map<std::string_view, Stop*> _stopname_to_stop;
        std::deque<Router> _routers;
        std::unordered_map<std::string_view, Router*> _busname_to_router;
        std::unordered_map<Stop*,std::unordered_map<Stop*, size_t>> _distance;

        size_t getDistance(Stop* from, Stop* to) const;

    };

}
