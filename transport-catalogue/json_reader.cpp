#include "json_reader.h"
#include "json.h"
#include "domain.h"
#include <algorithm>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_input
{
    using namespace json;
    using namespace std::literals;

     std::pair<const std::vector<domain::StopInfo>, const std::vector<domain::RouteInfo>> JsonReader::GetDbInfo()
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
}