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

    StopInfo::StopInfo(Stop &&stop, std::unordered_map<std::string, int> &&distances)
        : stop(std::move(stop)), distances(std::move(distances))
    {
    }
    RouteInfo::RouteInfo(std::string &&name, std::vector<std::string>&& stops, RouteType &&type)
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