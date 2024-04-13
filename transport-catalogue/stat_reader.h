#pragma once

#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"
namespace Catalogue {
    namespace Output {
        void ReadAndPrintStatRequest(size_t request_count, std::istream& input, std::ostream& output, const Data::TransportCatalogue &transport_catalogue);
        void ParseAndPrintStat(const Data::TransportCatalogue &tansport_catalogue, std::string_view request,
                               std::ostream &output);
        namespace Detail{

        std::string_view TrimSpaces(std::string_view string);

        size_t CountUniqueStops(const std::vector<Data::Stop *> &vector);

        double ComputeGeoLength(const std::vector<Data::Stop *> &route);

        }
    }
}