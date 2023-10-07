#include "request_handler.h"
#include <algorithm>

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace handlers
{
    void RequestHandler::AddInfo(const std::vector<domain::StopInfo> &stops,const std::vector<domain::RouteInfo> &routes){
        AddStopsInfo(stops);
        AddRouteInfo(routes);
    }

    void RequestHandler::AddStopsInfo(const std::vector<domain::StopInfo> &stops)
    {

        std::for_each(stops.begin(), stops.end(),
                      [&](auto &data)
                      {
                          db_.AddStop(data.stop);
                      });
        std::for_each(stops.begin(), stops.end(),
                      [&](auto &stop_data)
                      {
                          for (const auto &[next_stop, distance] : stop_data.distances)
                          {
                              db_.AddSegment(stop_data.stop.name, next_stop, distance);
                          }
                      });
    }
    void RequestHandler::AddRouteInfo(const std::vector<domain::RouteInfo> &routes)
    {
        std::for_each(routes.begin(), routes.end(),
                      [&](auto &route)
                      {
                          db_.AddRoute(route.name, route.stops, route.type);
                      });
    }

}