#pragma once
#include <unordered_map>
#include <string_view>
#include <string>
#include "geo.h"
#include <deque>
#include <vector>
#include <set>

namespace Catalogue{
    namespace Data {
        struct Stop {
            std::string stop_name;
            Geoposition::Coordinates pos;
        };

        struct Bus {
            std::string bus_name;
            std::vector<Stop *> stops_on_route;
        };

        class TransportCatalogue {
            // Реализуйте класс самостоятельно
        public:
            void Add_bus(Bus &&new_bus);

            void Add_stop(Stop &&new_stop);

            Stop *Find_stop(std::string_view stop_name) const;

            Bus *Find_bus(std::string_view bus_name) const;

            std::set<std::string_view> Find_buses_for_stop(std::string_view stop) const;

        private:
            std::deque<Bus> bus_data_;
            std::deque<Stop> stops_data_;
            std::unordered_map<std::string_view, Stop *> stops_;
            std::unordered_map<std::string_view, Bus *> buses_;
            std::unordered_map<Stop *, std::set<std::string_view>> buses_on_stop_;
        };
    }
}