#include "stat_reader.h"
#include "geo.h"
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <string>

namespace data_output
{

    void ProcessTcRequests(std::istream &in, transport_db::TransportCatalogue &tc)
    {
        int counter;
        (in >> counter).get();
        while (counter > 0)
        {
            ProcessRequest(in, tc);
            --counter;
        }
    }

    void ProcessRequest(std::istream &in, transport_db::TransportCatalogue &tc)
    {
        std::string request;
        std::getline(in, request);
        size_t pos = request.find(" ");
        std::string_view left = std::string_view(request).substr(0, pos);
        std::string_view right = std::string_view(request).substr(pos + 1);
        if (left == "Bus")
        {
            statistics::RouteStat(right, tc);
        }
        else if (left == "Stop")
        {
            statistics::StopStat(right, tc);
        }
    }
    namespace statistics
    {
        void StopStat(std::string_view request, transport_db::TransportCatalogue &tc)
        {
            if (tc.FindStop(request))
            {
                auto stop = tc.StopInfo(request);
                output_methods::PrintStopStat(request, stop->route_numbers);
            }
            else
            {
                output_methods::StopNotFound(request);
            }
        }
        void RouteStat(std::string_view request, transport_db::TransportCatalogue &tc)
        {
            if (tc.FindRote(request))
            {
                auto route = tc.RouteInfo(request);
                int stop_number, unique_stops;
                unique_stops = detail::CalcUnique(route->stops);
                stop_number = route->type == transport_db::RouteType::CIRCLE ? route->stops.size() : route->stops.size() * 2 - 1;
                double straight_route_len = detail::StraightRouteLen(route->stops, route->type);
                double real_route_len = detail::RealRouteLen(tc, route->stops, route->type);
                output_methods::PrintRouteStat(request, stop_number, unique_stops, real_route_len, real_route_len / straight_route_len);
            }
            else
            {
                output_methods::RouteNotFound(request);
            }
        }
        namespace detail
        {

            double StraightRouteLen(const std::vector<transport_db::Stop *> &stop_list, const transport_db::RouteType &route_type)
            {
                double route_len = 0;
                auto segment_start = stop_list.begin();
                auto segment_end = next(stop_list.begin());
                for (; segment_end != stop_list.end(); segment_end++)
                {
                    route_len += ComputeDistance((*segment_start)->coord, (*segment_end)->coord);
                    segment_start++;
                }
                route_len = route_type == transport_db::RouteType::LINEAR ? route_len * 2 : route_len;
                return route_len;
            }

            double RealRouteLen(transport_db::TransportCatalogue &tc, const std::vector<transport_db::Stop *> &stop_list, const transport_db::RouteType &route_type)
            {
                auto route_calc = [&](auto begin, auto end)
                {
                    double route_len = 0;
                    auto segment_start = begin;
                    auto segment_end = next(begin);
                    while (segment_end != end)
                    {
                        transport_db::Segment forward_segment(*segment_start, *segment_end);
                        transport_db::Segment backward_segment(*segment_end, *segment_start);
                        route_len += tc.FindSegment(forward_segment) ? tc.SegmentInfo(forward_segment) : tc.SegmentInfo(backward_segment);
                        segment_start++;
                        segment_end++;
                    }
                    transport_db::Segment end_circle(*std::prev(segment_end), *std::prev(segment_end));
                    route_len += route_type == transport_db::RouteType::LINEAR && tc.FindSegment(end_circle) ? tc.SegmentInfo(end_circle) : 0;
                    return route_len;
                };
                double forward_len = route_calc(stop_list.begin(), stop_list.end());
                double backward_len = route_type == transport_db::RouteType::LINEAR ? route_calc(stop_list.rbegin(), stop_list.rend()) : 0;
                return forward_len + backward_len;
            }

            int CalcUnique(const std::vector<transport_db::Stop *> &stops)
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
    namespace output_methods
    {
        void PrintStopStat(std::string_view stop_name, const std::set<std::string_view>& routes)
        {
            std::cout << "Stop " << stop_name << ":";
            std::string info = " buses";
            for (auto route_number : routes)
            {
                info += " " + (std::string)route_number;
            }
            std::cout << (info == " buses" ? " no buses" : info) << std::endl;
        }
        void StopNotFound(std::string_view requested_stop)
        {
            std::cout << "Stop " << requested_stop << ": not found" << std::endl;
        }

        void PrintRouteStat(std::string_view route, int stop_number, int unique_stops, double route_len, double curvature)
        {
            std::cout << "Bus " << route << ": " << stop_number << " stops on route, " << unique_stops << " unique stops, " << std::setprecision(6) << route_len << " route length, " << curvature << " curvature" << std::endl;
        }

        void RouteNotFound(std::string_view requested_route)
        {
            std::cout << "Bus " << requested_route << ": not found" << std::endl;
        }
    }
}