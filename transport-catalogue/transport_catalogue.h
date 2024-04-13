#pragma once
#include <unordered_map>
#include <string_view>
#include <string>
#include "geo.h"
#include <deque>
#include <vector>
#include <set>
#include <utility>

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

            void Add_distance_between_stops(std::string_view stop_from, std::string_view stop_to, int distance);

            Stop *Find_stop(std::string_view stop_name) const;

            Bus *Find_bus(std::string_view bus_name) const;

            std::set<std::string_view> Find_buses_for_stop(std::string_view stop) const;

            int ComputeRouteLength(const std::vector<Data::Stop *> &route) const;

        private:
            std::deque<Bus> bus_data_;
            std::deque<Stop> stops_data_;
            std::unordered_map<std::string_view, Stop *> stops_;
            std::unordered_map<std::string_view, Bus *> buses_;
            std::unordered_map<Stop *, std::set<std::string_view>> buses_on_stop_;
            struct Stops_pair_hasher{
                size_t operator()(std::pair<Stop*, Stop*> const p) const {
                    std::hash<const void*> ptr_hasher;
                    return (ptr_hasher(p.first) * 37) + (ptr_hasher(p.second) * 41);
                }
            };
            std::unordered_map< std::pair<Stop*, Stop*>, int, Stops_pair_hasher> distance_between_stops;

            int DisplayDistance(std::pair<Stop*, Stop*> stop_pair) const;
        };
    }
}