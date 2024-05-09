#include "transport_catalogue.h"
namespace catalogue {
    namespace data {
        void TransportCatalogue::AddBus(Bus &&new_bus) {
            bus_data_.push_back(std::move(new_bus));
            const auto& bus = bus_data_.back();
            buses_[std::string_view(bus.bus_name)] = &bus_data_.back();
            for(auto it = bus.stops_on_route.begin(); it != bus.stops_on_route.end(); ++it){
                if(!FindStop(*it)){
                    Stop tmp = {*it, {0.0, 0.0}, {}};
                    AddStop(std::move(tmp));
                }
                stops_.at(*it)->buses_on_stop.insert(bus.bus_name);
            }
        }

        void TransportCatalogue::AddStop(Stop &&new_stop) {
            bool stop_already_exist = false;
            for(auto& stop : stops_data_){
                if(stop.stop_name == new_stop.stop_name){
                    stop_already_exist = true;
                    break;
                }
            }
            if(!stop_already_exist){
                stops_data_.push_back(std::move(new_stop));
                stops_[std::string_view(stops_data_.back().stop_name)] = &stops_data_.back();
            }
            else{
                stops_.at(new_stop.stop_name)->pos = new_stop.pos;
                //stops_.at(new_stop.stop_name)->buses_on_stop = std::move(new_stop.buses_on_stop);
            }
        }

        void TransportCatalogue::AddDistanceBetweenStops(std::string_view from, std::string_view to, int distance){
            if(!FindStop(to)){
                Stop new_stop;
                new_stop.stop_name = to;
                new_stop.pos = {};
                new_stop.buses_on_stop = {};
                AddStop(std::move(new_stop));
            }
            auto from_stop = FindStop(from);
            auto to_stop = FindStop(to);
            distance_between_stops[{from_stop, to_stop}] = distance;
        }

        const Stop *TransportCatalogue::FindStop(std::string_view stop_name) const {
            if (!stops_.count(stop_name)) {
                return {};
            }
            return stops_.at(stop_name);
        }

        const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
            if (!buses_.count(bus_name)) {
                return {};
            }
            return buses_.at(bus_name);
        }

        int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
            if (from && to) {
                if (distance_between_stops.count({from, to})) {
                    return distance_between_stops.at({from, to});
                } else {
                    return distance_between_stops.at({to, from});
                }
            } else {
                return 0;
            }
        }

        const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::Get_all_buses() const {
            return buses_;
        }
    }
}