#include "transport_catalogue.h"
namespace Catalogue {
    namespace Data {
        void TransportCatalogue::Add_bus(Bus &&new_bus) {
            bus_data_.push_back(std::move(new_bus));
            buses_[std::string_view(bus_data_.back().bus_name)] = &bus_data_.back();
            for (auto &stop: bus_data_.back().stops_on_route) {
                buses_on_stop_[stop].insert(std::string_view(bus_data_.back().bus_name));
            }
        }

        void TransportCatalogue::Add_stop(Stop &&new_stop) {
            stops_data_.push_back(std::move(new_stop));
            stops_[std::string_view(stops_data_.back().stop_name)] = &stops_data_.back();
            buses_on_stop_[&stops_data_.back()];
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

        std::set<std::string_view> TransportCatalogue::Find_buses_for_stop(std::string_view stop) const {
            return buses_on_stop_.at(Find_stop(stop));
        }
    }
}