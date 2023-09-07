#pragma once
#include "input_reader.h"
#include <iomanip>
namespace output {
void WriteBusInFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue);
void WriteBus(const std::string& query, TransportCatalogue& catalogue);
void ParseQuery(const std::string& query, TransportCatalogue& catalogue);
void ParseQueryFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue);
void WriteStopInFile(std::ofstream& file, const std::string& query, TransportCatalogue& catalogue);
void WriteStop(const std::string& query, TransportCatalogue& catalogue);
}
