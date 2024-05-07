#include "request_handler.h"

RequestHandler::RequestHandler(const catalogue::data::TransportCatalogue &catalogue) :
catalogue_(catalogue)
{}

const std::optional<Bus *> RequestHandler::GetBusInfo(const std::string_view &bus_name) const {
    if(!catalogue_.Check_bus(bus_name)){
        return std::nullopt;
    }
    return catalogue_.Find_bus(bus_name);
}

const std::optional<Stop*> RequestHandler::GetStopInfo(const std::string_view &stop_name) const {
    if(!catalogue_.Check_stop(stop_name)){
        return std::nullopt;
    }
    return catalogue_.Find_stop(stop_name);
}

void RequestHandler::CalcDistance(double& geo_length, int& route_length, std::string_view stop_from, std::string_view stop_to){
    auto from = catalogue_.Find_stop(stop_from);
    auto to = catalogue_.Find_stop(stop_to);
    geo_length += catalogue::geoposition::ComputeDistance(from->pos, to->pos);
    route_length += catalogue_.GetDistance({from, to});
}

size_t RequestHandler::CountUniqueStops(const domain::Bus *bus) const {
    std::set<std::string_view> res;
    res.insert(bus->stops_on_route.begin(), bus->stops_on_route.end());
    return res.size();
}

std::vector<std::pair<Bus*, std::vector<Stop*>>> RequestHandler::GetDataForRender() {
    std::vector<std::pair<Bus*, std::vector<Stop*>>> result;

    const auto& buses = catalogue_.Get_all_buses();
    for(const auto& [bus_name, bus] : buses){
        std::vector<Stop*> stops;
        for(const auto& stop : bus->stops_on_route){
            stops.emplace_back(catalogue_.Find_stop(stop));
        }
        result.emplace_back(std::make_pair(bus, std::move(stops)));
    }
    auto ComparePairsByBusName = [](const std::pair<Bus*, std::vector<Stop*>>& lhs, const std::pair<Bus*, std::vector<Stop*>>& rhs) {
        return lhs.first->bus_name < rhs.first->bus_name;
    };
    std::sort(result.begin(), result.end(), ComparePairsByBusName);

    return result;
}
