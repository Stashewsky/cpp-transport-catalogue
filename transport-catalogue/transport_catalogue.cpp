#include "transport_catalogue.h"
namespace catalogue {
    namespace data {
        void TransportCatalogue::Add_bus(Bus &&new_bus) {
            bus_data_.push_back(std::move(new_bus));
            const auto& bus = bus_data_.back();
            buses_[std::string_view(bus.bus_name)] = &bus_data_.back();
            for(auto it = bus.stops_on_route.begin(); it != bus.stops_on_route.end(); ++it){
                if(!Check_stop(*it)){
                    Stop tmp = {*it, {0.0, 0.0}, {}};
                    Add_stop(std::move(tmp));
                }
                stops_.at(*it)->buses_on_stop.insert(bus.bus_name);
            }
        }

        void TransportCatalogue::Add_stop(Stop &&new_stop) {
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

        void TransportCatalogue::Add_distance_between_stops(Distance&& distance){
            if(!Check_stop(distance.to)){
                Stop new_stop;
                new_stop.stop_name = distance.to;
                new_stop.pos = {};
                new_stop.buses_on_stop = {};
                Add_stop(std::move(new_stop));
            }
            auto from = Find_stop(distance.from);
            auto to = Find_stop(distance.to);
            distance_between_stops[{from, to}] = distance.distance;
        }

        Stop *TransportCatalogue::Find_stop(std::string_view stop_name) const {
            if (!stops_.count(stop_name)) {
                return {};
            }
            return stops_.at(stop_name);
        }

        Bus* TransportCatalogue::Find_bus(std::string_view bus_name) const {
            if (!buses_.count(bus_name)) {
                return {};
            }
            return buses_.at(bus_name);
        }

        bool TransportCatalogue::Check_bus(std::string_view bus_name) const {
            return buses_.count(bus_name);
        }

        bool TransportCatalogue::Check_stop(std::string_view stop_name) const {
            return stops_.count(stop_name);
        }

        int TransportCatalogue::GetDistance(std::pair<Stop *, Stop *> stop_pair) const {
            if (stop_pair.first && stop_pair.second) {
                if (distance_between_stops.count(stop_pair)) {
                    return distance_between_stops.at(stop_pair);
                } else {
                    return distance_between_stops.at({stop_pair.second, stop_pair.first});
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