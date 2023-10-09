#include "map_renderer.h"
#include <functional>

namespace renderer
{
    namespace detail
    {
        bool LessThenError(double value)
        {
            return std::abs(value) < EPSILON;
        }
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    MapRenderer &MapRenderer::SetRouteData(std::set<std::string_view> &&routes)
    {
        route_list_ = routes;
        return *this;
    }

    MapRenderer &MapRenderer::SetStopData(std::vector<const domain::Stop*> &&stops){
        valid_stop_list_ = stops;
        return *this;
    }

    MapRenderer &MapRenderer::SetSettings(RenderSettings &&settings)
    {
        settings_ = settings;
        return *this;
    }

    SphereProjector MapRenderer::MakeProjector(const std::set<std::string_view> &route_list, const RenderSettings &settings)
    {
        std::vector<geo::Coordinates> all_stops;
        for (auto name : route_list)
        {
            const domain::Bus *route_info = db_.RouteInfo(name);
            for (auto stop : route_info->stops)
            {
                all_stops.push_back(stop->coord);
            }
        }
        return std::move(SphereProjector(all_stops.begin(), all_stops.end(), settings.width, settings.height, settings.padding));
    }

    void MapRenderer::PrepareRoutes(svg::Document &route_map, const SphereProjector &projector, const std::set<std::string_view> &route_list)
    {
        size_t route_number = 0;
        size_t color_palette_size = settings_.color_palette.size();
        for (auto name : route_list)
        {
            const domain::Bus *route_info = db_.RouteInfo(name);
            if (route_info->stops.size() == 0)
            {
                continue;
            }
            svg::Polyline route_line;
            route_line.SetStrokeColor(settings_.color_palette[route_number % color_palette_size]).SetStrokeWidth(settings_.line_width);
            route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetFillColor(svg::NoneColor);
            for (auto stop : route_info->stops)
            {
                route_line.AddPoint(projector(stop->coord));
            }
            if (route_info->type == domain::RouteType::LINEAR)
            {
                auto stops_it = std::next(route_info->stops.rbegin());
                while (stops_it != route_info->stops.rend())
                {
                    route_line.AddPoint(projector((*stops_it)->coord));
                    ++stops_it;
                }
            }
            route_map.Add(route_line);
            ++route_number;
        }
    }

    void MapRenderer::PrepareRouteNumbers(svg::Document &route_map, const SphereProjector &projector, const std::set<std::string_view> &route_list)
    {
        size_t route_number = 0;
        size_t color_palette_size = settings_.color_palette.size();
        for (auto name : route_list)
        {
            const domain::Bus *route_info = db_.RouteInfo(name);
            if (route_info->stops.size() == 0)
            {
                continue;
            }
            auto stop_label = MakeCaption(projector(route_info->stops.at(0)->coord), route_info->stops.at(0)->name, settings_.color_palette[route_number % color_palette_size]);
            route_map.Add(stop_label.first);
            route_map.Add(stop_label.second);
            if (route_info->type == domain::RouteType::LINEAR)
            {
                auto stop_label = MakeCaption(projector(route_info->stops.back()->coord), route_info->stops.at(0)->name, settings_.color_palette[route_number % color_palette_size]);
                route_map.Add(stop_label.first);
                route_map.Add(stop_label.second);
            }
            ++route_number;
        }
    }

    void MapRenderer::PrepareStopSymbols(svg::Document &route_map, const SphereProjector &projector, const std::set<std::string_view> &route_list)
    {
        
        for(auto route:route_list);
    }
    void MapRenderer::PrepareStopNames(svg::Document &route_map, const SphereProjector &projector, const std::set<std::string_view> &route_list)
    {
    }

    std::pair<svg::Text, svg::Text> MapRenderer::MakeCaption(const svg::Point location, const std::string &name, const svg::Color &color)
    {
        svg::Text underlayer;
        underlayer.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg::Text caption;
        caption.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetFillColor(color);
        return {underlayer, caption};
    }
}
