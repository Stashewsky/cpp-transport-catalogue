#pragma once
#include <unordered_map>
#include <string_view>
#include <deque>
#include <utility>
#include "domain.h"

namespace catalogue{
    namespace data {
        using namespace domain;

        class TransportCatalogue {
        public:
            void Add_bus(Bus &&new_bus);
            void Add_stop(Stop &&new_stop);
            void Add_distance_between_stops(Distance&& distance);
            Stop *Find_stop(std::string_view stop_name) const;
            Bus *Find_bus(std::string_view bus_name) const;
            bool Check_bus(std::string_view bus_name) const;
            bool Check_stop(std::string_view stop_name)const;
            int GetDistance(std::pair<Stop*, Stop*> stop_pair) const;
            const std::unordered_map<std::string_view, Bus*>& Get_all_buses() const;
        private:
            std::deque<Bus> bus_data_;
            std::deque<Stop> stops_data_;
            std::unordered_map<std::string_view, Stop *> stops_;
            std::unordered_map<std::string_view, Bus *> buses_;
            struct Stops_pair_hasher{
                size_t operator()(std::pair<Stop*, Stop*> const p) const {
                    std::hash<const void*> ptr_hasher;
                    return (ptr_hasher(p.first) * 37) + (ptr_hasher(p.second) * 41);
                }
            };
            std::unordered_map< std::pair<Stop*, Stop*>, int, Stops_pair_hasher> distance_between_stops;
        };
    }
}