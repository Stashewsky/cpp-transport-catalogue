#pragma once
#include "geo.h"
#include <string>
#include <vector>
#include <set>
namespace domain {

    struct Stop {
        std::string stop_name;
        catalogue::geoposition::Coordinates pos;
        std::set<std::string> buses_on_stop;
    };

    struct Bus {
        std::string bus_name;
        std::vector<std::string> stops_on_route;
        bool is_circle_route = true;
    };

    struct Distance{
        std::string from;
        std::string to;
        int distance;
    };
}