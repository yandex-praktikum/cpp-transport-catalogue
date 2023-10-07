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

        JsonReader &ReadDocument()
        {
            Document encoded_json = json::Load(in_);
            root_ = std::move(encoded_json.GetRoot().AsMap());
            return *this;
        };

        JsonReader &InitDB()
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
                handler_.AddInfo(raw_stop_data, raw_route_data);
            }
            return *this;
        }

        template <typename OutStream>
        void ProcessRequests(OutStream &out)
        {
            if (root_.count("stat_requests"s))
            {
                const Array &requests = root_.at("stat_requests"s).AsArray();
                for (const Node &node : requests)
                {
                    Dict request = node.AsMap();
                    if (request.at("type"s).AsString() == "Stop"s)
                    {
                        auto stop_info = handler_.GetStopInfo(request.at("name").AsString());
                        if (stop_info.has_value())
                        {
                            Array buses;
                            for (auto route_number : stop_info.value())
                            {
                                buses.emplace_back(Node{std::string(route_number)});
                            }
                            json::Document responce(Dict{{"request_id", request.at("id"s)},
                                                         {"buses", Node{buses}}});
                            json::Print(responce, out);
                        }
                        else
                        {
                            InfoNotFound(request.at("id"s), out);
                        }
                    }
                    else
                    {
                        auto route_info = handler_.GetRouteStat(request.at("name").AsString());
                        if (route_info.has_value())
                        {
                            domain::RouteStat info = route_info.value();
                            json::Document responce(Dict{{"curvature"s, Node{info.curvature}},
                                                         {"request_id"s, request.at("id"s)},
                                                         {"route_length"s, Node{static_cast<int>(info.length)}},
                                                         {"stop_count", Node{info.stop_count}},
                                                         {"unique_stop_count", Node{info.unique_stop_count}}});
                            json::Print(responce, out);
                        }
                        else
                        {
                            InfoNotFound(request.at("id"s), out);
                        }
                    }
                }
            }
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

        template <typename OutStream>
        void InfoNotFound(Node &request_id, OutStream &out)
        {
            json::Document not_found(Dict{{"request_id", request_id},
                                          {"error_message", Node{"not found"s}}});
            json::Print(not_found, out);
        }
    };
}