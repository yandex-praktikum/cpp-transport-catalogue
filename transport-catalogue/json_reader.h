#pragma once
#include "json.h"
#include "domain.h"
#include "request_handler.h"
#include <algorithm>

namespace json_input
{

    using namespace json;
    using namespace std::literals;

    class JsonReader
    {
    public:
        explicit JsonReader(handlers::RequestHandler *handler)
            : handler_(handler){};

        template <typename InStream>
        JsonReader &ReadDocument(InStream &in)
        {
            Document encoded_json = json::Load(in);
            root_ = std::move(encoded_json.GetRoot().AsMap());
            return *this;
        };

        template <typename OutStream>
        JsonReader &ProcessRequests(OutStream &out)
        {
            if (root_.count("stat_requests"s))
            {
                Array all_answers;
                const Array &requests = root_.at("stat_requests"s).AsArray();
                for (const Node &node : requests)
                {
                    Dict request = node.AsMap();
                    if (request.at("type"s).AsString() == "Stop"s)
                    {
                        auto stop_info = handler_->GetStopInfo(request.at("name").AsString());
                        if (stop_info.has_value())
                        {
                            Array buses;
                            for (auto route_number : stop_info.value())
                            {
                                buses.emplace_back(Node{std::string(route_number)});
                            }
                            all_answers.emplace_back(Node{Dict{{"request_id", request.at("id"s)},
                                                               {"buses", Node{buses}}}});
                        }
                        else
                        {
                            all_answers.emplace_back(Node{Dict{{"request_id", request.at("id"s)},
                                                               {"error_message", Node{"not found"s}}}});
                        }
                    }
                    else if (request.at("type"s).AsString() == "Bus"s)
                    {
                        auto route_info = handler_->GetRouteStat(request.at("name").AsString());
                        if (route_info.has_value())
                        {
                            domain::RouteStat info = route_info.value();
                            all_answers.emplace_back(Node{Dict{{"curvature"s, Node{info.curvature}},
                                                               {"request_id"s, request.at("id"s)},
                                                               {"route_length"s, Node{static_cast<int>(info.length)}},
                                                               {"stop_count", Node{info.stop_count}},
                                                               {"unique_stop_count", Node{info.unique_stop_count}}}});
                        }
                        else
                        {
                            all_answers.emplace_back(Node{Dict{{"request_id", request.at("id"s)},
                                                               {"error_message", Node{"not found"s}}}});
                        }
                    }
                    else
                    {
                        all_answers.emplace_back(Node{Dict{{"request_id", request.at("id"s)},
                                                           {"map", Node{handler_->GetMap()}}}});
                    }
                }
                json::Document result(Node{all_answers});
                json::Print(result, out);
            }
            return *this;
        }

        std::pair<const std::vector<domain::StopInfo>, const std::vector<domain::RouteInfo>> GetDbInfo();
        renderer::RenderSettings GetRenderSettings();

    private:
        handlers::RequestHandler *handler_;
        Dict root_;
        std::set<std::string> existing_routes_;

        domain::StopInfo ProcessStopInfo(Dict &&request)
        {
            domain::Stop stop;
            stop.name = request.at("name").AsString();
            stop.coord = geo::Coordinates{request.at("latitude").AsDouble(), request.at("longitude").AsDouble()}; // вынести в handler&
            std::unordered_map<std::string, int> distances;
            for (auto &[next_stop, distance] : request.at("road_distances").AsMap())
            {
                distances[next_stop] = distance.AsInt();
            }
            return {std::move(stop), std::move(distances)};
        };

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

        svg::Color DecodeColorValue(Node &node)
        {
            if (node.IsString())
            {
                return std::move(svg::Color{node.AsString()});
            }
            else
            {
                Array color = node.AsArray();
                if (color.size() == 3)
                {
                    return std::move(svg::Rgb{static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt())});
                }
                else
                {
                    return std::move(svg::Rgba{static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt()), color[3].AsDouble()});
                }
            }
        };
    };
}