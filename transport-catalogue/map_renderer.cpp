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

    MapRenderer &MapRenderer::SetRouteData(std::vector<const domain::Bus *> &&routes)
    {
        route_list_ = routes;
        return *this;
    }

    MapRenderer &MapRenderer::SetStopData(std::vector<const domain::Stop *> &&stops)
    {
        valid_stop_list_ = stops;
        return *this;
    }

    MapRenderer &MapRenderer::SetSettings(RenderSettings &&settings)
    {
        settings_ = settings;
        return *this;
    }

    SphereProjector MapRenderer::MakeProjector()
    {
        std::vector<geo::Coordinates> all_stops;
        for (auto stop : valid_stop_list_)
        {
            all_stops.push_back(stop->coord);
        }
        return std::move(SphereProjector(all_stops.begin(), all_stops.end(), settings_.width, settings_.height, settings_.padding));
    }

    void MapRenderer::PrepareRoutes(svg::Document &route_map, const SphereProjector &projector)
    {
        size_t route_number = 0;
        size_t color_palette_size = settings_.color_palette.size();
        for (auto route : route_list_)
        {

            svg::Polyline route_line;
            route_line.SetStrokeColor(settings_.color_palette[route_number % color_palette_size]).SetStrokeWidth(settings_.line_width);
            route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetFillColor(svg::NoneColor);
            for (auto stop : route->stops)
            {
                route_line.AddPoint(projector(stop->coord));
            }
            if (route->type == domain::RouteType::LINEAR)
            {
                auto stops_it = std::next(route->stops.rbegin());
                while (stops_it != route->stops.rend())
                {
                    route_line.AddPoint(projector((*stops_it)->coord));
                    ++stops_it;
                }
            }
            route_map.Add(route_line);
            ++route_number;
        }
    }

    void MapRenderer::PrepareRouteNumbers(svg::Document &route_map, const SphereProjector &projector)
    {
        size_t route_number = 0;
        size_t color_palette_size = settings_.color_palette.size();
        for (auto route : route_list_)
        {
            auto stop_label = MakeRouteCaption(projector(route->stops.at(0)->coord), route->route_number, settings_.color_palette[route_number % color_palette_size]);
            route_map.Add(stop_label.first);
            route_map.Add(stop_label.second);
            if (route->type == domain::RouteType::LINEAR && route->stops.at(0)!=route->stops.at(route->stops.size()-1))
            {
                auto stop_label = MakeRouteCaption(projector(route->stops.back()->coord), route->route_number, settings_.color_palette[route_number % color_palette_size]);
                route_map.Add(stop_label.first);
                route_map.Add(stop_label.second);
            }
            ++route_number;
        }
    }

    void MapRenderer::PrepareStopSymbols(svg::Document &route_map, const SphereProjector &projector)
    {
        for (auto stop : valid_stop_list_)
        {
            svg::Circle stop_mark;
            stop_mark.SetCenter(projector(stop->coord)).SetRadius(settings_.stop_radius).SetFillColor("white"s);
            route_map.Add(stop_mark);
        }
    }

    void MapRenderer::PrepareStopNames(svg::Document &route_map, const SphereProjector &projector)
    {
        for (auto stop : valid_stop_list_)
        {
            auto stop_caption = MakeStopCaption(projector(stop->coord),stop->name);
            route_map.Add(stop_caption.first);
            route_map.Add(stop_caption.second);
        }
    }

    std::pair<svg::Text, svg::Text> MapRenderer::MakeRouteCaption(const svg::Point location, const std::string &name, const svg::Color &color)
    {
        svg::Text underlayer;
        underlayer.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg::Text caption;
        caption.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetFillColor(color);
        return {underlayer, caption};
    }

    std::pair<svg::Text, svg::Text> MapRenderer::MakeStopCaption(const svg::Point location, const std::string &name)
    {
        svg::Text underlayer;
        underlayer.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.stop_label_offset[0], settings_.stop_label_offset[1]}).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg::Text caption;
        caption.SetData(name).SetPosition(location).SetOffset(svg::Point{settings_.stop_label_offset[0], settings_.stop_label_offset[1]}).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetFillColor("black"s);
        return {underlayer, caption};
    }
}
