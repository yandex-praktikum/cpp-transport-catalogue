#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "stat_reader.h"
#include <iostream>

// #define DEBUG

#ifdef DEBUG
#include <fstream>
#include "log_duration.h"
#endif

int main()
{

#ifndef DEBUG
    transport_db::TransportCatalogue tc;

    handlers::RequestHandler handler(tc);
    json_input::JsonReader reader(std::cin, handler);
    reader.ReadDocument().InitDB();
    reader.ProcessRequests(std::cout);

    //data_output::ProcessTcRequests(std::cin, tc);
#endif

#ifdef DEBUG
    TransportCatalogue tc;
    std::fstream in("tsC_case1_input.txt");
    {
        LOG_DURATION(std::cout);
        input::ReadData(in, tc);
    }
    // output::ProcessTcRequests(in, tc);
    in.close();
#endif
}