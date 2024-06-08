#include "transport_router.h"
namespace catalogue {
    namespace routes {
        TransportRouter::TransportRouter(const data::TransportCatalogue &catalogue, RouteSettings &settings)
                : catalogue_(catalogue),
                  settings_(settings),
                  stops_(catalogue_.GetAllStops()),
                  buses_(catalogue_.Get_all_buses()){
            SetGraph();
            router_ = std::make_unique<Router<double>>(*graph_);
        }

        void TransportRouter::SetGraph() {
            graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue_.GetAllStops().size() * 2);
            SetStops();
            AddEdgeToStop();
            AddEdgeToBus();
        }

        std::optional<StopVertexPair> TransportRouter::GetVertexPairByStop(const Stop *stop) const {
            if (stop_vertex_map_.count(stop)) {
                return stop_vertex_map_.at(stop);
            } else {
                return std::nullopt;
            }
        }

        std::optional<RouteInfo> TransportRouter::GetRouteInfo(VertexId start, graph::VertexId end) const {
            const auto &route_info = router_->BuildRoute(start, end);
            if (route_info) {
                RouteInfo result;
                result.total_time = route_info->weight;

                for (const auto edge: route_info->edges) {
                    result.edges.emplace_back(GetEdge(edge));
                }

                return result;

            } else {
                return std::nullopt;
            }
        }

        std::variant<StopEdge, BusEdge> TransportRouter::GetEdge(EdgeId id) const {
            return edges_data_.at(id);
        }

        void TransportRouter::SetStops(){
            size_t i = 0;

            for (const auto& [stop_name, stop_ptr] : stops_) {
                VertexId first = i++;
                VertexId second = i++;

                stop_vertex_map_[stop_ptr] = StopVertexPair{first, second};
            }
        }

        void TransportRouter::AddEdgeToStop() {
            for (const auto& [stop, num] : stop_vertex_map_) {
                EdgeId id = graph_->AddEdge(Edge<double>{num.from,
                                                         num.to,
                                                         settings_.wait_time});

                edges_data_[id] = StopEdge{stop->stop_name, settings_.wait_time};
            }
        }

        void TransportRouter::AddEdgeToBus() {
            for (const auto& [bus_name, bus_ptr] : buses_) {
                ParseBusForEdge(bus_ptr->stops_on_route.begin(),
                                bus_ptr->stops_on_route.end(),
                                bus_ptr);

                if (!bus_ptr->is_circle_route) {
                    ParseBusForEdge(bus_ptr->stops_on_route.rbegin(),
                                    bus_ptr->stops_on_route.rend(),
                                    bus_ptr);
                }
            }
        }

        Edge<double> TransportRouter::MakeBusEdge(const Stop* start, const Stop* end, const double distance) const {
            Edge<double> result;

            result.from = stop_vertex_map_.at(start).to;
            result.to = stop_vertex_map_.at(end).from;
            result.weight = distance * 1.0 / (settings_.velocity * KILOMETER / HOUR);

            return result;
        }
    }
}