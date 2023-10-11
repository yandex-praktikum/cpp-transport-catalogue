#include "request_handler.h"
#include <algorithm>
#include <unordered_set>
#include <execution>
#include "domain.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace handlers
{
    using namespace domain;

    void RequestHandler::InitDB()
    {
        auto db_fill_requests = reader_->GetDbInfo();
        AddStopsInfo(std::move(db_fill_requests.first));
        AddRouteInfo(std::move(db_fill_requests.second));
    }

    void RequestHandler::SetReader(json_reader::JsonReader *reader)
    {
        reader_ = reader;
    }

    void RequestHandler::SetMapRenderer(renderer::MapRenderer *render_ptr)
    {
        map_renderer_ = render_ptr;
    }

    void RequestHandler::AddStopsInfo(const std::vector<std::unique_ptr<domain::RawRequest>> &&stops)
    {

        std::for_each(stops.begin(), stops.end(),
                      [&](auto &data)
                      {
                          domain::AddStopRequest *stop_data = dynamic_cast<domain::AddStopRequest *>(data.get());
                          Stop stop;
                          stop.name= std::move(stop_data->GetStopName());
                          stop.coord = std::move(stop_data->GetCoords());
                          db_.AddStop(stop);
                      });
        std::for_each(stops.begin(), stops.end(),
                      [&](auto &data)
                      {
                          domain::AddStopRequest *stop_data = dynamic_cast<domain::AddStopRequest *>(data.get());
                          for (const auto &[next_stop, distance] : stop_data->GetDistances())
                          {
                              db_.AddSegment(stop_data->GetStopName(), next_stop, distance);
                          }
                      });
    }
    void RequestHandler::AddRouteInfo(const std::vector<std::unique_ptr<domain::RawRequest>> &&routes)
    {
        std::for_each(routes.begin(), routes.end(),
                      [&](auto &route)
                      {
                        domain::AddRouteRequest *route_data = dynamic_cast<domain::AddRouteRequest *>(route.get());
                          db_.AddRoute(route_data->GetRouteName(), route_data->GetStopsList(), route_data->GetType());
                      });
    }

    std::unique_ptr<domain::Answer> RequestHandler::GetRouteStat(const std::string_view &route_name, int request_id) const
    {
        if (db_.FindRote(route_name))
        {
            const Bus *route = db_.RouteInfo(route_name);
            RouteStat statistic;
            int real_len = static_cast<int>(detail::RealRouteLen(db_, route->stops, route->type));
            double curvature = real_len / detail::StraightRouteLen(route->stops, route->type);
            int stop_count = route->type == RouteType::CIRCLE ? route->stops.size() : route->stops.size() * 2 - 1;
            int unique_stop_count = detail::CalcUnique(route->stops);
            return std::move(std::make_unique<domain::RouteInfoAnswer>(domain::RouteInfoAnswer{domain::AnswerType::ROUTE_INFO,
                                                                                               request_id,
                                                                                               (std::string)route_name,
                                                                                               curvature,
                                                                                               real_len,
                                                                                               stop_count,
                                                                                               unique_stop_count}));
        }
        else
        {
            return std::move(std::make_unique<domain::ErrorAnswer>(domain::ErrorAnswer{domain::AnswerType::ERROR, request_id}));
        }
    }

    std::unique_ptr<domain::Answer> RequestHandler::GetStopInfo(const std::string_view &stop_name, int request_id) const
    {
        if (db_.FindStop(stop_name))
        {
            auto routes = db_.StopInfo(stop_name)->route_numbers;
            return std::move(std::make_unique<domain::StopInfoAnswer>(domain::StopInfoAnswer{domain::AnswerType::STOP_INFO, request_id, routes}));
        }
        return std::move(std::make_unique<domain::ErrorAnswer>(domain::ErrorAnswer{domain::AnswerType::ERROR, request_id}));
    }

    std::unique_ptr<domain::Answer> RequestHandler::GetMap(int request_id)
    {
        std::ostringstream out;
        out << ""s;
        if (map_renderer_)
        {
            map_renderer_.value()->SetRouteData(GetValidRoutes());
            map_renderer_.value()->SetStopData(GetValidStops());
            map_renderer_.value()->Render(out);
        }
        return std::move(std::make_unique<domain::MapContentAnswer>(domain::MapContentAnswer{domain::AnswerType::MAP, request_id, out.str()}));
    }

    std::vector<const domain::Bus *> RequestHandler::GetValidRoutes()
    {
        auto all_routes = std::move(db_.GetAllRoutes());
        std::vector<const domain::Bus *> out;
        for (auto route : all_routes)
        {
            auto route_info = db_.RouteInfo(route);
            if (route_info->stops.size())
            {
                out.push_back(route_info);
            }
        }
        return out;
    }

    std::vector<const domain::Stop *> RequestHandler::GetValidStops()
    {
        std::set<const domain::Stop *> valid_stops;
        for (auto route : db_.GetAllRoutes())
        {
            for (auto stop : db_.RouteInfo(route)->stops)
            {
                valid_stops.insert(stop);
            }
        }
        std::vector<const domain::Stop *> out(valid_stops.begin(), valid_stops.end());
        std::sort(std::execution::par, out.begin(), out.end(), [](auto &left, auto &right)
                  { return left->name < right->name; });
        return out;
    }

    namespace detail
    {
        using namespace domain;

        double StraightRouteLen(const std::vector<Stop *> &stop_list, const RouteType &route_type)
        {
            double route_len = 0;
            auto segment_start = stop_list.begin();
            auto segment_end = next(stop_list.begin());
            for (; segment_end != stop_list.end(); segment_end++)
            {
                route_len += geo::ComputeDistance((*segment_start)->coord, (*segment_end)->coord);
                segment_start++;
            }
            route_len = route_type == RouteType::LINEAR ? route_len * 2 : route_len;
            return route_len;
        }

        double RealRouteLen(const transport_db::TransportCatalogue &tc, const std::vector<Stop *> &stop_list, const RouteType &route_type)
        {
            auto route_calc = [&](auto begin, auto end)
            {
                double route_len = 0;
                auto segment_start = begin;
                auto segment_end = next(begin);
                while (segment_end != end)
                {
                    Segment forward_segment(*segment_start, *segment_end);
                    Segment backward_segment(*segment_end, *segment_start);
                    route_len += tc.FindSegment(forward_segment) ? tc.SegmentInfo(forward_segment) : tc.SegmentInfo(backward_segment);
                    segment_start++;
                    segment_end++;
                }
                Segment end_circle(*std::prev(segment_end), *std::prev(segment_end));
                route_len += route_type == RouteType::LINEAR && tc.FindSegment(end_circle) ? tc.SegmentInfo(end_circle) : 0;
                return route_len;
            };
            double forward_len = route_calc(stop_list.begin(), stop_list.end());
            double backward_len = route_type == RouteType::LINEAR ? route_calc(stop_list.rbegin(), stop_list.rend()) : 0;
            return forward_len + backward_len;
        }

        int CalcUnique(const std::vector<domain::Stop *> &stops)
        {
            std::unordered_set<std::string_view> unique_ptrs;
            for (auto stop : stops)
            {
                unique_ptrs.insert(stop->name);
            }
            return unique_ptrs.size();
        }
    }

}