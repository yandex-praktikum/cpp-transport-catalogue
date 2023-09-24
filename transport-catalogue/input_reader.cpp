#include "input_reader.h"

using RouteData = std::pair<std::vector<std::string>, transport_db::RouteType>;

namespace data_input
{
    StopInfo::StopInfo(transport_db::Stop &&stop, std::unordered_map<std::string, int> &&distances)
        : stop(std::move(stop)), distances(std::move(distances))
    {
    }
    RouteInfo::RouteInfo(std::string &&name, std::vector<std::string> stops, transport_db::RouteType &&type)
        : name(std::move(name)), stops(std::move(stops)), type(std::move(type))
    {
    }
    namespace detail
    {

        std::pair<std::string_view, std::string_view> Split(std::string_view in, char delimeter)
        {
            size_t pos = in.find(delimeter);
            std::string_view left = in.substr(0, pos);
            std::string_view right = in.substr(pos + 1);
            return {left, right};
        }

        std::vector<std::string_view> MultiSplit(std::string_view in, char delimeter)
        {
            std::vector<std::string_view> out;
            while (in.find(delimeter) != in.npos)
            {
                auto [left, right] = Split(in, delimeter);
                out.push_back(left);
                in = right;
            }
            out.push_back(in);
            return out;
        }
        double CoordDecode(std::string_view coords)
        {
            return std::stod((std::string)coords);
        }

        std::pair<int, std::string_view> DistanceDecode(std::string_view in)
        {
            auto [distance_str, stop_name] = Split(in, 'm');
            stop_name.remove_prefix(4);
            return {std::stoi((std::string)distance_str), stop_name};
        }

    }

    namespace process_request
    {
        StopInfo StopRequest(std::string_view line)
        {
            transport_db::Stop out;
            std::unordered_map<std::string, int> distances;
            auto [stop_name, right] = detail::Split(line, ':');
            out.name = (std::string)stop_name;

            std::vector<std::string_view> StopInfoText = detail::MultiSplit(right, ',');
            out.coord = {detail::CoordDecode(StopInfoText[0]), detail::CoordDecode(StopInfoText[1])};
            for (size_t idx = 2; idx < StopInfoText.size(); idx++)
            {
                auto [distance, next_stop] = detail::DistanceDecode(StopInfoText[idx]);
                distances[(std::string)next_stop] = distance;
            }
            StopInfo result{std::move(out), std::move(distances)};
            return result;
        }

        RouteInfo RouteRequest(std::string_view line)
        {
            std::vector<std::string> stops;
            char delimeter = line.find('>') == line.npos ? '-' : '>';
            auto [route_num, route] = detail::Split(line, ':');
            auto route_stops = detail::MultiSplit(route, delimeter);
            std::for_each(route_stops.begin(), std::prev(route_stops.end()), [&stops](std::string_view stop_name)
                          { stop_name.remove_prefix(1);
                      stop_name.remove_suffix(1);
                      stops.push_back(std::move((std::string)stop_name)); });
            route_stops[route_stops.size()-1].remove_prefix(1);
            stops.push_back(std::move((std::string)route_stops[route_stops.size() - 1]));
            transport_db::RouteType type = delimeter == '>' ? transport_db::RouteType::CIRCLE : transport_db::RouteType::LINEAR;
            RouteInfo out{std::move((std::string)route_num), std::move(stops), std::move(type)};
            return out;
        }
    }
}
