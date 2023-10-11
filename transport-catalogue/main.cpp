#include "transport_catalogue.h"
//#include "json_reader.h"
#include "request_handler.h"
#include <iostream>


int main()
{

    transport_db::TransportCatalogue tc;
    json_reader::JsonReader reader;
    renderer::MapRenderer renderer;
    handlers::RequestHandler handler(tc);
    handler.SetReader(&reader);
    handler.SetMapRenderer(&renderer);
    handler.ReadData(std::cin);
    handler.ProcessRequests(std::cout);
    
    
    // reader.ReadDocument(std::cin);
    // renderer.SetSettings(reader.GetRenderSettings());
    // handler.SetMapRenderer(&renderer);
    // handler.InitDB(reader.GetDbInfo());
    // handler.ProcessRequests(std::cout);
    
        
}