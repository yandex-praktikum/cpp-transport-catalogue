#pragma once
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include <optional>
#include <sstream>
#include <memory>

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
        
        RequestHandler(transport_db::TransportCatalogue &db)
            : db_(db){};

        //---------------init section-----------
        
        
        

        void SetReader(json_reader::JsonReader *reader);

        void SetMapRenderer(renderer::MapRenderer *render_ptr);

        template <typename InStream>
        void ReadData(InStream &in)
        {
            reader_->ReadDocument(in);
            InitDB();
        };
        //---------------Process requests section----------
        template <typename OutStream>
        void ProcessRequests(OutStream &out)
        {
            auto requests = reader_->GetRequests();
            std::vector<std::unique_ptr<domain::Answer>> answers;
            for (auto request_it = requests.begin(); request_it != requests.end(); request_it++)
            {
                if (request_it->get()->GetType() == domain::RequestType::ROUTE_QUERY)
                {
                    domain::GetRouteRequest *request = dynamic_cast<domain::GetRouteRequest *>(request_it->get());
                    answers.push_back(std::move(GetRouteStat(request->GetRouteName(), request->GetId())));
                }
                else if (request_it->get()->GetType() == domain::RequestType::STOP_QUERY)
                {
                    domain::GetStopRequest *request = dynamic_cast<domain::GetStopRequest *>(request_it->get());
                    answers.push_back(std::move(GetStopInfo(request->GetStopName(), request->GetId())));
                }
                else
                {
                    domain::RenderMapRequest *request = dynamic_cast<domain::RenderMapRequest *>(request_it->get());
                    map_renderer_.value()->SetSettings(reader_->GetRenderSettings());
                    answers.push_back(std::move(GetMap(request->GetId())));
                }
            }
            reader_->ProcessAnswers(std::move(answers), out);
        }

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_db::TransportCatalogue &db_;
        json_reader::JsonReader *reader_;
        std::optional<renderer::MapRenderer *> map_renderer_;
        void InitDB();
        std::unique_ptr<domain::Answer> GetRouteStat(const std::string_view &bus_name, int request_id) const;
        std::unique_ptr<domain::Answer> GetStopInfo(const std::string_view &stop_name, int request_id) const;
        std::vector<const domain::Bus *> GetValidRoutes();
        std::vector<const domain::Stop *> GetValidStops();
        std::unique_ptr<domain::Answer> GetMap(int request_id);
        void AddStopsInfo(const std::vector<std::unique_ptr<domain::RawRequest>> &&stops);
        void AddRouteInfo(const std::vector<std::unique_ptr<domain::RawRequest>> &&routes);

    };

    namespace detail
    {
        double StraightRouteLen(const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);

        double RealRouteLen(const transport_db::TransportCatalogue &tc, const std::vector<domain::Stop *> &stop_list, const domain::RouteType &route_type);

        int CalcUnique(const std::vector<domain::Stop *> &stops);
    }
}
