#include "json_reader.h"
#include "json.h"
#include "domain.h"
#include <algorithm>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader
{
    using namespace json;
    using namespace std::literals;

     std::pair<const std::vector<std::unique_ptr<domain::RawRequest>>, const std::vector<std::unique_ptr<domain::RawRequest>>> JsonReader::GetDbInfo()
        {
            std::vector<std::unique_ptr<domain::RawRequest>> raw_stop_data;
            std::vector<std::unique_ptr<domain::RawRequest>> raw_route_data;
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
                return {std::move(raw_stop_data), std::move(raw_route_data)};
            }
            return {};
        }

    renderer::RenderSettings JsonReader::GetRenderSettings()
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

    std::unique_ptr<domain::RawRequest> JsonReader::ProcessStopInfo(Dict &&request)
        {
            
            std::string name = request.at("name").AsString();
            geo::Coordinates coord = geo::Coordinates{request.at("latitude").AsDouble(), request.at("longitude").AsDouble()}; // вынести в handler&
            std::unordered_map<std::string, int> distances;
            for (auto &[next_stop, distance] : request.at("road_distances").AsMap())
            {
                distances[next_stop] = distance.AsInt();
            }
            return std::move(std::make_unique<domain::AddStopRequest>(domain::AddStopRequest{domain::RequestType::ADD_STOP, std::move(name), std::move(coord), std::move(distances)}));
        }

        std::unique_ptr<domain::RawRequest> JsonReader::ProcessRouteInfo(Dict &&request)
        {
            std::vector<std::string> stops;
            std::string route_name = std::move(request.at("name").AsString());
            for (auto &stop : request.at("stops").AsArray())
            {
                stops.push_back(std::move(stop.AsString()));
            }
            domain::RouteType type = request.at("is_roundtrip").AsBool() ? domain::RouteType::CIRCLE : domain::RouteType::LINEAR;
            return std::move(std::make_unique<domain::AddRouteRequest>(domain::AddRouteRequest{domain::RequestType::ADD_ROUTE, std::move(route_name), std::move(stops), std::move(type)}));;
        }

         svg::Color JsonReader::DecodeColorValue(Node &node)
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
        }
}

