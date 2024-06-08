#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json.h"
namespace catalogue {
    namespace detail {
        namespace json {
            class JSON_Reader{
            public:
                void ParseJSONdocument(Document& doc);
                void ExecuteBaseRequests(TransportCatalogue &catalogue);
                map_renderer::RenderSettings GetRenderSettings();
                Document ExecuteStatRequests(RequestHandler& handler);
            private:
                void ExecuteStopRequest(const Node& stop_node, TransportCatalogue &catalogue);
                void ExecuteBusRequest(const Node& bus_node, TransportCatalogue &catalogue);
                Node MakeStopResponse(const Node& stop_node, RequestHandler& handler);
                Node MakeBusResponse(const Node& bus_node, RequestHandler& handler);
                Node MakeMapResponse(const Node& map_node, RequestHandler& handler);
                Node MakeRouteResponse(const Node& route_node, RequestHandler& handler);
                svg::Color ParseColor(const Node& color_node);
                Node base_requests_;
                Node stat_requests_;
                Node render_settings_;
                Node routing_settings_;
            };
        }
    }
}