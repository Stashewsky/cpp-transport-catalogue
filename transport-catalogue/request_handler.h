#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <optional>
#include <map>
using namespace catalogue::data;
class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& catalogue);

    const std::optional<Bus*> GetBusInfo(const std::string_view& bus_name) const;
    const std::optional<Stop*> GetStopInfo(const std::string_view &stop_name) const;
    void CalcDistance(double& geo_length, int& route_length, std::string_view stop_from, std::string_view stop_to);
    size_t CountUniqueStops(const Bus* bus) const;
    std::vector<std::pair<Bus*, std::vector<Stop*>>> GetDataForRender();
private:
    const TransportCatalogue& catalogue_;
};
