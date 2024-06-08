#pragma once
#include "geo.h"
#include "graph.h"
#include <string>
#include <vector>
#include <variant>
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

    struct RouteSettings{
        double wait_time = 0;
        double velocity = 0.0;
    };

    struct StopEdge{
        std::string_view name;
        double time = 0;
    };

    struct BusEdge{
        std::string_view bus_name;
        size_t span_count = 0;
        double time = 0;
    };

    struct StopVertexPair{
        graph::VertexId from;
        graph::VertexId to;
    };

    struct RouteInfo {
        double total_time = 0.0;
        std::vector<std::variant<StopEdge, BusEdge>> edges;
    };
}