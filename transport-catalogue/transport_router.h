#pragma once
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include <optional>
#include <memory>
#include <algorithm>

namespace catalogue {
    namespace routes {
        using namespace domain;
        using namespace graph;

        static const int KILOMETER = 1000;
        static const int HOUR = 60;

        class TransportRouter {
        public:
            TransportRouter(const data::TransportCatalogue& catalogue, RouteSettings& settings);
            std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;
        private:
            const data::TransportCatalogue& catalogue_;

            std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
            std::unique_ptr<Router<double>> router_;

            RouteSettings settings_;
            const std::unordered_map<std::string_view, Stop*>& stops_;
            const std::unordered_map<std::string_view, Bus*>& buses_;

            std::unordered_map<const Stop*, StopVertexPair> stop_vertex_map_;
            std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edges_data_;

            void SetGraph();
            std::optional<StopVertexPair> GetVertexPairByStop(const Stop* stop) const;
            std::variant<StopEdge, BusEdge> GetEdge(EdgeId id) const;
            void SetStops();
            void AddEdgeToStop();
            void AddEdgeToBus();
            Edge<double> MakeBusEdge(const Stop* start, const Stop* end, const double distance) const;

            template <typename Iterator>
            void ParseBusForEdge(Iterator first, Iterator last, const Bus* bus) {

                for (auto it = first; it != last; ++it) {
                    size_t distance = 0;
                    size_t span = 0;
                    const auto& current_stop = stops_.at(*it);

                    for (auto it2 = std::next(it); it2 != last; ++it2) {
                        const auto& from = stops_.at(*prev(it2));
                        const auto& to = stops_.at(*it2);
                        distance += catalogue_.GetDistance(from, to);
                        ++span;

                        EdgeId id = graph_->AddEdge(MakeBusEdge(current_stop, to, distance));

                        edges_data_[id] = BusEdge{bus->bus_name, span, graph_->GetEdge(id).weight};
                    }
                }
            }
        };
    }
}