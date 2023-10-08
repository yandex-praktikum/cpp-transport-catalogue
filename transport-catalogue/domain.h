#pragma once

#include <vector>
#include <set>
#include <string>
#include <cstdint>
#include "geo.h"
#include <string>
#include <unordered_map>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain
{

    enum class RouteType
    {
        CIRCLE,
        LINEAR
    };

    class Stop
    {
    public:
        std::string name;
        geo::Coordinates coord;
        std::set<std::string_view> route_numbers;
        size_t Hash() const;

    private:
        std::hash<int64_t> hasher_;
    };

    using Segment = std::pair<Stop *, Stop *>;

    class StopHasher
    {
    public:
        size_t operator()(const Segment &segment) const;
    };

    struct Bus
    {
        std::string route_number;
        std::vector<Stop *> stops;
        RouteType type;
    };

    struct StopInfo
    {
        StopInfo(Stop &&stop, std::unordered_map<std::string, int> &&distances);
        Stop stop;
        std::unordered_map<std::string, int> distances;
    };

    struct RouteInfo
    {
        RouteInfo(std::string &&name, std::vector<std::string> &&stops, RouteType &&type);
        std::string name;
        std::vector<std::string> stops;
        RouteType type;
    };

    struct Request
    {
        int id;
        std::string type;
        std::string body;
    };

    struct RouteStat
    {
        double curvature;
        int length;
        int stop_count;
        int unique_stop_count;
    };

}
