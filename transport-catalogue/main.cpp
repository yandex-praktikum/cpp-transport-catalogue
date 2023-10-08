#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include <iostream>

#ifdef DEBUG
#include "log_duration.h"
#endif

int main()
{

    transport_db::TransportCatalogue tc;
    handlers::RequestHandler handler(tc);
    json_input::JsonReader reader(std::cin, handler);
    renderer::MapRenderer renderer(tc);
    reader.ReadDocument();
    handler.InitDB(reader.GetDbInfo());
    // reader.ProcessRequests(std::cout);
    renderer.SetSettings(reader.GetRenderSettings())
        .SetRouteData(handler.GetRoutes())
        .Render(std::cout);
}