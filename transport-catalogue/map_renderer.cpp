#include "map_renderer.h"

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

    void MapRenderer::PrepareRoutes(svg::Document& route_map, const SphereProjector projector, const std::set<std::string_view> &route_list)
    {
        size_t route_number = 0;
        size_t color_palette_size = settings_.color_palette.size();
        for (auto name : route_list)
        {
            const domain::Bus *route_info = db_.RouteInfo(name);
            if(route_info->stops.size()==0){
                continue;
            }
            svg::Polyline route_line;
            route_line.SetStrokeColor(settings_.color_palette[route_number % color_palette_size]).SetStrokeWidth(settings_.line_width);
            route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetFillColor(svg::NoneColor);
            for (auto stop : route_info->stops)
            {
                route_line.AddPoint(projector(stop->coord));
                std::cout<<stop->name<<std::endl;
            }
            if (route_info->type == domain::RouteType::LINEAR)
            {
                auto stops_it = std::next(route_info->stops.rbegin());
                while (stops_it != route_info->stops.rend())
                {
                    route_line.AddPoint(projector((*stops_it)->coord));
                    std::cout << (*stops_it)->name << std::endl;
                    ++stops_it;
                }
            }
                route_map.Add(route_line);
                ++route_number;
        }
    }
}
