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
            void AddBus(Bus &&new_bus);
            void AddStop(Stop &&new_stop);
            void AddDistanceBetweenStops(std::string_view from, std::string_view to, int distance);
            Stop *FindStop(std::string_view stop_name) const;
            Bus *FindBus(std::string_view bus_name) const;
            int GetDistance(Stop* from, Stop* to) const;
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