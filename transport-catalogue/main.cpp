#include <iostream>


#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"
#include "serialization.h"








#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {

        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        json_input::Parser parser;
        TransportCatalogue catalogue;
        //std::ifstream in ("input.txt");

        auto document = json::Load(std::cin);
        auto root = document.GetRoot().AsDict();
        auto input = root["base_requests"].AsArray();
        parser.LoadInputQueries(input, catalogue);
        auto serialize_info = root.at("serialization_settings").AsDict();
        auto rend_info = root["render_settings"].AsDict();
        auto router_info = root.at("routing_settings").AsDict();
        Serialize(serialize_info.at("file").AsString(), catalogue, rend_info, router_info);


    } else if (mode == "process_requests"sv) {
        auto document = json::Load(std::cin);
        auto root = document.GetRoot().AsDict();
        auto serialize_info = root.at("serialization_settings").AsDict();
        auto file = serialize_info.at("file").AsString();

        transport_catalogue_serialize::CommonInfo info;
        std::ifstream input(file, std::ios::binary);

        if (!info.ParseFromIstream(&input)) {
            std::cerr <<"ERROr";
        }

        auto catalogue = DeserializeCatalogue(info.catalogue());
        json::Dict rend_info = DeserializeSetting(info.render_settings());
        auto [time, velocity] = DeserializeRouter(info.router());

        //std::fstream out("output.txt");
        auto output = root["stat_requests"].AsArray();
        TransportRouter router(catalogue, velocity, time);
        auto ans = json_output::LoadOutputQueries(output, catalogue,rend_info, router);
        json::Print(json::Document{ans}, std::cout);

        

    } else {
        PrintUsage();
        return 1;
    }
}
