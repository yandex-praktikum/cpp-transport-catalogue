#pragma once

#include <vector>
#include <set>
#include <string>
#include <cstdint>
#include "geo.h"
#include <string>
#include <unordered_map>
#include <optional>

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

    //--------------- request classes-----------

    enum class RequestType
    {
        ADD_STOP,
        ADD_ROUTE,
        STOP_QUERY,
        ROUTE_QUERY,
        RENDER_MAP
    };

    class RawRequest
    {
    public:
        explicit RawRequest(RequestType &&type);
        virtual ~RawRequest() = default;

        RequestType GetType();
        void SetId(int id);
        int GetId() const;

    private:
        RequestType type_;
        std::optional<int> request_id_;
    };

    class AddStopRequest : public RawRequest
    {
    public:
        explicit AddStopRequest(RequestType type, const std::string &&stop_name, const geo::Coordinates &&coord, std::unordered_map<std::string, int> &&distances);
          
        std::string GetStopName();
        geo::Coordinates GetCoords();
        std::unordered_map<std::string, int> GetDistances();

    private:
        std::string name_;
        geo::Coordinates coords_;
        std::unordered_map<std::string, int> distances_;
    };

    class AddRouteRequest : public RawRequest
    {
    public:
        explicit AddRouteRequest(RequestType type, std::string &&route_name, std::vector<std::string> &&stops, RouteType &&route_type);

        std::string GetRouteName();
        std::vector<std::string> GetStopsList();
        RouteType GetType();

    private:
        std::string name_;
        std::vector<std::string> stops_;
        RouteType route_type_;
    };

    class GetRouteRequest : public RawRequest
    {
    public:
       explicit GetRouteRequest(RequestType type, std::string &&route_name, int id);

        std::string GetRouteName();

    private:
        std::string name_;
    };

    class GetStopRequest : public RawRequest
    {
    public:
       explicit GetStopRequest(RequestType type, std::string &&stop_name, int id);
            
        std::string GetStopName();

    private:
        std::string name_;
    };

    class RenderMapRequest : public RawRequest
    {
    public:
       explicit RenderMapRequest(RequestType type, int id);
    };

    //-----------------answer classes------------

    enum class AnswerType
    {
        STOP_INFO,
        ROUTE_INFO,
        MAP,
        ERROR

    };

    class Answer
    {
    public:
        explicit Answer(AnswerType &&type, int id);
        virtual ~Answer() = default;

        int GetId();
        AnswerType GetType();

    private:
        AnswerType type_;
        int id_;
    };

    class StopInfoAnswer : public Answer
    {
    public:
        explicit StopInfoAnswer(AnswerType type, int id, std::set<std::string_view> routes);

        std::string GetName();
        std::set<std::string_view> GetRoutes();

    private:
        std::string name_;
        std::set<std::string_view> routes_;
    };

    class RouteInfoAnswer : public Answer
    {
    public:
        explicit RouteInfoAnswer(AnswerType type, int id, std::string name, double curvature, int route_length, int stop_count, int unique_stop_count);

        std::string GetName();
        double GetCurv();
        int GetLen();
        int GetStops();
        int GetUniqueStops();

    private:
        std::string name_;
        double curvature_;
        int route_length_;
        int stop_count_;
        int unique_stop_count_;
    };

    class MapContentAnswer : public Answer
    {
    public:
        explicit MapContentAnswer(AnswerType type, int id, std::string &&rendered_map);
        std::string GetMap();
    private:
        std::string map_;
    };

    class ErrorAnswer : public Answer
    {
    public:
        explicit ErrorAnswer(AnswerType type, int id);

        std::string GetError();

    private:
        std::string error_ = "not found";
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

    struct RouteStat
    {
        double curvature;
        int length;
        int stop_count;
        int unique_stop_count;
    };

}
