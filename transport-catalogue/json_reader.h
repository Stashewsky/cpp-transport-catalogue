#pragma once
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json.h"
namespace catalogue {
    namespace detail {
        namespace json {
            class JSON_Reader{
            public:
                void Parse_JSON_document(Document& doc);
                void Execute_base_requests(TransportCatalogue& catalogue);
                map_renderer::RenderSettings Get_render_settings();
                Document Execute_stat_requests(RequestHandler& handler);
            private:
                void Execute_STOP_request(const Node& stop_node, TransportCatalogue& catalogue);
                void Execute_BUS_request(const Node& bus_node, TransportCatalogue& catalogue);
                Dict MakeStopResponse(const Node& stop_node, RequestHandler& handler);
                Dict MakeBusResponse(const Node& bus_node, RequestHandler& handler);
                Dict MakeMapResponse(const Node& map_node, RequestHandler& handler);
                svg::Color ParseColor(const Node& color_node);
                Node base_requests_;
                Node stat_requests_;
                Node render_settings_;
            };
        }
    }
}