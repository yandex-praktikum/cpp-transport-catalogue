#pragma once

#include <istream>
#include <iomanip>
#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"

namespace TransportCatalogueStat{

    class stat_reader
    {
    public:
        stat_reader(std::istream&, TransportCatalogue::transport_catalogue&);
    private:
        void printRouterInfo(const TransportCatalogue::RouterInfo&) const;
        void printStopInfo(const TransportCatalogue::StopInfo&) const;
    };
}
