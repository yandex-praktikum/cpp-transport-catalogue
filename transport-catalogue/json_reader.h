#pragma once
#include "json.h"
#include "sstream"
#include "transport_catalogue.h"
#include "domain.h"
#include "request_handler.h"
#include "map_renderer.h"
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

        std::pair<const std::vector<domain::StopInfo>, const std::vector<domain::RouteInfo>> GetDbInfo()
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
                return {raw_stop_data, raw_route_data};
            }
            return {};
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
                        auto stop_info = handler_.GetStopInfo(request.at("name").AsString());
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
                    else
                    {
                        auto route_info = handler_.GetRouteStat(request.at("name").AsString());
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
                }
                json::Document result(Node{all_answers});
                json::Print(result, out);
            }
            return *this;
        }

        renderer::RenderSettings GetRenderSettings()
        {
            using namespace renderer;
            RenderSettings render_settings;
            if (root_.count("render_settings"s))
            {
                Dict raw_settings = root_.at("render_settings"s).AsMap();
                render_settings.height = raw_settings.at("height"s).AsDouble();
                render_settings.width = raw_settings.at("width"s).AsDouble();
                render_settings.padding = raw_settings.at("padding"s).AsDouble();
                render_settings.line_width = raw_settings.at("line_width"s).AsDouble();
                render_settings.stop_radius = raw_settings.at("stop_radius"s).AsDouble();
                render_settings.bus_label_font_size = raw_settings.at("bus_label_font_size"s).AsInt();
                render_settings.stop_label_font_size = raw_settings.at("stop_label_font_size"s).AsInt();
                render_settings.underlayer_width = raw_settings.at("underlayer_width"s).AsDouble();
                Array bus_label_offset = raw_settings.at("bus_label_offset"s).AsArray();
                render_settings.bus_label_offset = std::move(std::vector<double>{bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()});
                Array stop_label_offset = raw_settings.at("stop_label_offset"s).AsArray();
                render_settings.stop_label_offset = std::move(std::vector<double>{stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()});
                render_settings.underlayer_color = DecodeColorValue(raw_settings.at("underlayer_color"));
                Array palette = raw_settings.at("color_palette").AsArray();
                for (Node &node : palette)
                {
                    render_settings.color_palette.push_back(DecodeColorValue(node));
                }
            }
            return render_settings;
        }

    private:
        InStream &in_;
        handlers::RequestHandler &handler_;
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