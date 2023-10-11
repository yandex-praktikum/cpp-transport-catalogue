#pragma once
#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include <algorithm>
#include <memory>

namespace json_reader
{

    using namespace json;
    using namespace std::literals;

    class JsonReader
    {
    public:
        JsonReader() = default;


        template <typename InStream>
        JsonReader &ReadDocument(InStream &in)
        {
            Document encoded_json = json::Load(in);
            root_ = std::move(encoded_json.GetRoot().AsMap());
            return *this;
        }

        std::vector<std::unique_ptr<domain::RawRequest>> GetRequests();

        template <typename OutStream>
        void ProcessAnswers(std::vector<std::unique_ptr<domain::Answer>> in, OutStream &out)
        {
            json::Array answers;
            for (auto answer_ptr = in.begin(); answer_ptr != in.end(); answer_ptr++)
            {
                if (answer_ptr->get()->GetType() == domain::AnswerType::STOP_INFO)
                {
                    domain::StopInfoAnswer *stop_info_ptr = dynamic_cast<domain::StopInfoAnswer *>(answer_ptr->get());
                    Array buses;
                    for (auto route_number : stop_info_ptr->GetRoutes())
                    {
                        buses.emplace_back(Node{std::string(route_number)});
                    }
                    answers.emplace_back(Node{Dict{{"request_id", Node{stop_info_ptr->GetId()}},
                                                   {"buses", Node{buses}}}});
                }
                else if (answer_ptr->get()->GetType() == domain::AnswerType::ROUTE_INFO)
                {
                    domain::RouteInfoAnswer *route_info_ptr = dynamic_cast<domain::RouteInfoAnswer *>(answer_ptr->get());
                    answers.emplace_back(Node{Dict{{"curvature"s, Node{route_info_ptr->GetCurv()}},
                                                   {"request_id"s, Node{route_info_ptr->GetId()}},
                                                   {"route_length"s, Node{route_info_ptr->GetLen()}},
                                                   {"stop_count", Node{route_info_ptr->GetStops()}},
                                                   {"unique_stop_count", Node{route_info_ptr->GetUniqueStops()}}}});
                }
                else if (answer_ptr->get()->GetType() == domain::AnswerType::MAP)
                {
                    domain::MapContentAnswer *map_ptr = dynamic_cast<domain::MapContentAnswer *>(answer_ptr->get());
                    answers.emplace_back(Node{Dict{{"request_id", Node{map_ptr->GetId()}},
                                                   {"map", Node{map_ptr->GetMap()}}}});
                }
                else
                {
                    domain::ErrorAnswer *error_ptr = dynamic_cast<domain::ErrorAnswer *>(answer_ptr->get());
                    answers.emplace_back(Node{Dict{{"request_id", Node{error_ptr->GetId()}},
                                                   {"error_message", Node{error_ptr->GetError()}}}});
                }
            }
            json::Document result(Node{answers});
            json::Print(result, out);
        }

        std::pair<const std::vector<std::unique_ptr<domain::RawRequest>>, const std::vector<std::unique_ptr<domain::RawRequest>>> GetDbInfo();

        renderer::RenderSettings GetRenderSettings();

    private:
        Dict root_;
        std::set<std::string> existing_routes_;
        std::unique_ptr<domain::RawRequest> ProcessStopInfo(Dict &&request);
        std::unique_ptr<domain::RawRequest> ProcessRouteInfo(Dict &&request);
        svg::Color DecodeColorValue(Node &node);
        
    };
}