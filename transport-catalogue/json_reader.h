#pragma once
#include "json.h"
#include "sstream"
#include "transport_catalogue.h"
#include "domain.h"
#include "request_handler.h"
#include <algorithm>

namespace json_input
{

    using namespace json;
    using namespace std::literals;

    template <typename InStream>
    class JsonReader
    {
    public:
        explicit JsonReader(InStream &in, handlers::RequestHandler &handler)
            : in_(in), handler_(handler){};

        JsonReader& ReadDocument()
        {
            Document encoded_json = json::Load(in_);
            root_ = std::move(encoded_json.GetRoot().AsMap());
            return *this;
        };

        JsonReader& InitDB()
        {
            std::vector<domain::StopInfo> raw_stop_data;
            std::vector<domain::RouteInfo> raw_route_data;
            if (root_.count("base_requests"s))
            {
                Array base_request_ = std::move(root_.at("base_requests"s).AsArray());
                for (Node &node : base_request_)
                {
                    Dict request = node.AsMap();
                    if (request.at("type"s).AsString() == "Stop"s)
                    {
                        raw_stop_data.push_back(std::move(ProcessStopInfo(std::move(request))));
                    }
                    else
                    {
                        raw_route_data.push_back(std::move(ProcessRouteInfo(std::move(request))));
                    }
                }
                handler_.AddInfo(raw_stop_data,raw_route_data);
            }
            return *this;
        }

        void ProcessRequests(){

        }

    private:
        InStream &in_;
        handlers::RequestHandler &handler_;
        Dict root_;

         domain::StopInfo ProcessStopInfo(Dict &&request)
        {
            domain::Stop stop;
            stop.name = request.at("name").AsString();
            stop.coord = Coordinates{request.at("latitude").AsDouble(), request.at("longitude").AsDouble()};
            std::unordered_map<std::string, int> distances;
            for (auto &[next_stop, distance] : request.at("road_distances").AsMap())
            {
                distances[next_stop] = distance.AsInt();
            }
            return {std::move(stop), std::move(distances)};
        }

        domain::RouteInfo ProcessRouteInfo(Dict &&request)
        {
            std::vector<std::string> stops;
            std::string route_name = std::move(request.at("name").AsString());
            for (auto &stop : request.at("stops").AsArray())
            {
                stops.push_back(std::move(stop.AsString()));
            }
            domain::RouteType type = request.at("is_roundtrip").AsBool() ? domain::RouteType::CIRCLE : domain::RouteType::LINEAR;
            return {std::move(route_name), std::move(stops), std::move(type)};
        };
    };
}