#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include <iostream>


int main()
{

    transport_db::TransportCatalogue tc;
    handlers::RequestHandler handler(tc);
    json_input::JsonReader reader(&handler);
    renderer::MapRenderer renderer;
    reader.ReadDocument(std::cin);
    renderer.SetSettings(reader.GetRenderSettings());
    handler.SetMapRenderer(&renderer);
    handler.InitDB(reader.GetDbInfo());
    reader.ProcessRequests(std::cout);
    
        
}