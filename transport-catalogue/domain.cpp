#include "domain.h"
#include <unordered_map>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
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

    RawRequest::RawRequest(RequestType &&type)
        : type_(type){}

    RequestType RawRequest::GetType()
    {
        return type_;
    }
    void RawRequest::SetId(int id)
    {
        request_id_ = id;
    }
    int RawRequest::GetId() const
    {
        return request_id_ ? request_id_.value() : 0;
    }

    AddStopRequest::AddStopRequest(RequestType type, const std::string &&stop_name, const geo::Coordinates &&coord, std::unordered_map<std::string, int> &&distances)
        : RawRequest(std::move(type)), name_(stop_name), coords_(coord), distances_(distances)
    {
    }

    std::string AddStopRequest::GetStopName()
    {
        return name_;
    }
    geo::Coordinates AddStopRequest::GetCoords()
    {
        return coords_;
    }
    std::unordered_map<std::string, int> AddStopRequest::GetDistances()
    {
        return distances_;
    }

    AddRouteRequest::AddRouteRequest(RequestType type, std::string &&route_name, std::vector<std::string> &&stops, RouteType &&route_type)
        : RawRequest(std::move(type)), name_(route_name), stops_(stops), route_type_(route_type)
    {
    }
    std::string AddRouteRequest::GetRouteName()
    {
        return name_;
    }
    std::vector<std::string> AddRouteRequest::GetStopsList()
    {
        return stops_;
    }
    RouteType AddRouteRequest::GetType()
    {
        return route_type_;
    }

    GetRouteRequest::GetRouteRequest(RequestType type, std::string &&route_name, int id)
        : RawRequest(std::move(type)), name_(route_name)
    {
        SetId(id);
    }

    std::string GetRouteRequest::GetRouteName()
    {
        return name_;
    }

    GetStopRequest::GetStopRequest(RequestType type, std::string &&stop_name, int id)
        : RawRequest(std::move(type)), name_(stop_name)
    {
        SetId(id);
    }

    std::string GetStopRequest::GetStopName()
    {
        return name_;
    }

    RenderMapRequest::RenderMapRequest(RequestType type, int id)
        : RawRequest(std::move(type))
    {
        SetId(id);
    }

    Answer::Answer(AnswerType &&type, int id)
        : type_(type), id_(id){}
    int Answer::GetId()
    {
        return id_;
    }
    AnswerType Answer::GetType()
    {
        return type_;
    }

    StopInfoAnswer::StopInfoAnswer(AnswerType type, int id, std::set<std::string_view> routes)
        : Answer(std::move(type), id), routes_(std::move(routes)){}
    std::string StopInfoAnswer::GetName()
    {
        return std::move(name_);
    }
    std::set<std::string_view> StopInfoAnswer::GetRoutes()
    {
        return std::move(routes_);
    }

    RouteInfoAnswer::RouteInfoAnswer(AnswerType type, int id, std::string name, double curvature, int route_length, int stop_count, int unique_stop_count)
        : Answer(std::move(type), id), name_(std::move(name)), curvature_(curvature), route_length_(route_length),
          stop_count_(stop_count), unique_stop_count_(unique_stop_count){}

    std::string RouteInfoAnswer::GetName()
    {
        return std::move(name_);
    }
    double RouteInfoAnswer::GetCurv()
    {
        return curvature_;
    }
    int RouteInfoAnswer::GetLen()
    {
        return route_length_;
    }
    int RouteInfoAnswer::GetStops()
    {
        return stop_count_;
    }
    int RouteInfoAnswer::GetUniqueStops()
    {
        return unique_stop_count_;
    }

    MapContentAnswer::MapContentAnswer(AnswerType type, int id, std::string &&rendered_map)
        : Answer(std::move(type), id), map_(rendered_map){}
    std::string MapContentAnswer::GetMap()
    {
        return std::move(map_);
    }

    ErrorAnswer::ErrorAnswer(AnswerType type, int id)
        : Answer(std::move(type), id){}
    std::string ErrorAnswer::GetError()
    {
        return std::move(error_);
    }

    StopInfo::StopInfo(Stop &&stop, std::unordered_map<std::string, int> &&distances)
        : stop(std::move(stop)), distances(std::move(distances))
    {
    }
    RouteInfo::RouteInfo(std::string &&name, std::vector<std::string> &&stops, RouteType &&type)
        : name(std::move(name)), stops(std::move(stops)), type(std::move(type))
    {
    }

    size_t Stop::Hash() const
    {
        return hasher_((int64_t)this);
    }

    size_t StopHasher::operator()(const Segment &segment) const
    {
        size_t hash_value = segment.first->Hash() * 37;
        hash_value += segment.second->Hash() * 37 * 37 * 37;
        return hash_value;
    }
}