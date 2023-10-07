#pragma once

#include "transport_catalogue.h"
#include <optional>

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace handlers
{
    class RequestHandler
    {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
        // RequestHandler(const transport_db::TransportCatalogue& db, const renderer::MapRenderer& renderer);
        RequestHandler(transport_db::TransportCatalogue &db)
            : db_(db){};

        //-----------db fill requests---------
        void AddInfo(const std::vector<domain::StopInfo> &stops, const std::vector<domain::RouteInfo> &routes);
        //-----------------------------------
        //
        //--------- requests answers --------
        std::optional<domain::RouteStat> GetRouteStat(const std::string_view &bus_name) const;

        std::optional<std::set<std::string_view>> GetStopInfo(const std::string_view& stop_name) const;
        //----------------------------------
        // Этот метод будет нужен в следующей части итогового проекта
        // svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_db::TransportCatalogue &db_;

        void AddStopsInfo(const std::vector<domain::StopInfo> &stops);
        void AddRouteInfo(const std::vector<domain::RouteInfo> &routes);
        // const renderer::MapRenderer& renderer_;
    };

    namespace detail
    {
        double StraightRouteLen(const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);

        double RealRouteLen(const transport_db::TransportCatalogue &tc, const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);
     
        int CalcUnique(const std::vector<domain::Stop *> &stops);
    }
}
