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
        explicit RawRequest(RequestType &&type)
            : type_(type)
        {
        }
        virtual ~RawRequest() = default;

        RequestType GetType()
        {
            return type_;
        }
        void SetId(int id)
        {
            request_id_ = id;
        }
        std::optional<int> GetId()
        {
            return request_id_;
        }

    private:
        RequestType type_;
        std::optional<int> request_id_;
    };

    class AddStopRequest : public RawRequest
    {
    public:
        AddStopRequest(RequestType type, const std::string &&stop_name, const geo::Coordinates &&coord, std::unordered_map<std::string, int> &&distances)
            : RawRequest(std::move(type)), name_(stop_name), coords_(coord), distances_(distances)
        {
        }
        std::string GetStopName()
        {
            return name_;
        }
        geo::Coordinates GetCoords()
        {
            return coords_;
        }
        std::unordered_map<std::string, int> GetDistances()
        {
            return distances_;
        }

    private:
        std::string name_;
        geo::Coordinates coords_;
        std::unordered_map<std::string, int> distances_;
    };

    class AddRouteRequest : public RawRequest
    {
    public:
        AddRouteRequest(RequestType type, std::string &&route_name, std::vector<std::string> &&stops, RouteType &&route_type)
            : RawRequest(std::move(type)), name_(route_name), stops_(stops), route_type_(route_type)
        {
        }
        std::string GetStopName()
        {
            return name_;
        }
        std::vector<std::string> GetStopsList()
        {
            return stops_;
        }
        RouteType GetDistances()
        {
            return route_type_;
        }

    private:
        std::string name_;
        std::vector<std::string> stops_;
        RouteType route_type_;
    };

    class GetRouteRequest : public RawRequest
    {
    public:
        GetRouteRequest(RequestType type, std::string &&route_name, int id)
            : RawRequest(std::move(type)), name_(route_name)
        {
            SetId(id);
        }
        std::string GetStopName()
        {
            return name_;
        }

    private:
        std::string name_;
    };

    class GetStopRequest : public RawRequest
    {
    public:
        GetStopRequest(RequestType type, std::string &&stop_name, int id)
            : RawRequest(std::move(type)), name_(stop_name)
        {
            SetId(id);
        }
        std::string GetStopName()
        {
            return name_;
        }

    private:
        std::string name_;
    };

    class RenderMapRequest : public RawRequest
    {
    public:
        RenderMapRequest(RequestType type, int id)
            : RawRequest(std::move(type))
        {
            SetId(id);
        }
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
        explicit Answer(AnswerType &&type, int id)
            : type_(type), id_(id){};
        virtual ~Answer() = default;
        int GetId()
        {
            return id_;
        }

        AnswerType GetType()
        {
            return type_;
        }

    private:
        int id_;
        AnswerType type_;
    };

    class StopInfoAnswer : public Answer
    {
    public:
        StopInfoAnswer(AnswerType type, int id)
            : Answer(std::move(type), id){};

        std::string GetName()
        {
            return std::move(name_);
        }

        std::set<std::string_view> GetRoutes()
        {
            return std::move(routes_);
        }
        void AddRoute(std::string_view route)
        {
            routes_.insert(route);
        }

    private:
        std::string name_;
        std::set<std::string_view> routes_;
    };

    class RouteInfoAnswer : public Answer
    {
    public:
        RouteInfoAnswer(AnswerType type, int id)
            : Answer(std::move(type), id){};

        std::string GetName()
        {
            return std::move(name_);
        }

        double GetCurv()
        {
            return curvature_;
        }
        int GetLen()
        {
            return route_length_;
        }
        int GetStops()
        {
            return stop_count_;
        }
        int GetUniqueStops()
        {
            return unique_stop_count_;
        }

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
        MapContentAnswer(AnswerType type,  int id, std::string &&rendered_map)
            : Answer(std::move(type), id), map_(rendered_map){};

        std::string GetMap()
        {
            return std::move(map_);
        }

    private:
        std::string map_;
    };

    class ErrorAnswer : public Answer
    {
    public:
        ErrorAnswer(AnswerType type, int id)
            : Answer(std::move(type), id){};

        std::string GetError()
        {
            return std::move(error_);
        }

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

    struct Request // нужен ли?
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
