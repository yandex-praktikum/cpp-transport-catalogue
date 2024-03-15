#pragma once
#include "input_reader.h"
#include <iomanip>
namespace output {

void WriteBus(std::ostream& out,const std::string& query,  TransportCatalogue& catalogue);

void ParseOutput(std::ostream& out, const std::string& query,  TransportCatalogue& catalogue);


void WriteStop(std::ostream& out, const std::string& query, TransportCatalogue& catalogue);

void LoadOutputQueries(std::istream& in, std::ostream& out, TransportCatalogue& catalogue);
}
