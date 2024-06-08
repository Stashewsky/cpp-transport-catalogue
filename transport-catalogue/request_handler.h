#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include <optional>
#include <map>
using namespace catalogue::data;
class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& catalogue);

    const Bus * GetBusInfo(const std::string_view& bus_name) const;
    const Stop * GetStopInfo(const std::string_view &stop_name) const;
    void CalcDistance(double& geo_length, int& route_length, std::string_view stop_from, std::string_view stop_to);
    size_t CountUniqueStops(const Bus* bus) const;
    std::vector<std::pair<const Bus*, std::vector<const Stop*>>> GetDataForRender();
    std::optional<RouteInfo> GetRouteInformation(std::string_view from, std:: string_view to, RouteSettings& settings);
private:
    const TransportCatalogue& catalogue_;
    std::unique_ptr<catalogue::routes::TransportRouter> router_;
};
