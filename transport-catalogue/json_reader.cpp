#include "json_reader.h"
#include <sstream>
#include "json_builder.h"
namespace catalogue {
    namespace detail {
        namespace json {
            void JSON_Reader::ParseJSONdocument(Document &doc) {
                const auto &root_map = doc.GetRoot().AsMap();
                //нужно ли тут и далее по коду проверять каждую Ноду на соответствие типу перед "извлечением"?
                base_requests_ = root_map.at("base_requests");
                stat_requests_ = root_map.at("stat_requests");
                render_settings_ = root_map.at("render_settings");
                routing_settings_ = root_map.at("routing_settings");
            }

            void JSON_Reader::ExecuteBaseRequests(TransportCatalogue &catalogue) {
                std::string_view request_type;
                for (const auto &elem: base_requests_.AsArray()) {
                    request_type = elem.AsMap().at("type").AsString();

                    if (request_type == "Stop") {
                        ExecuteStopRequest(elem, catalogue);
                    } else if (request_type == "Bus") {
                        ExecuteBusRequest(elem, catalogue);
                    }
                }
            }

            void JSON_Reader::ExecuteStopRequest(const Node &stop_node,
                                                 TransportCatalogue &catalogue) {
                const auto &stop = stop_node.AsMap();
                Stop new_stop;
                std::string stop_name = stop.at("name").AsString();
                new_stop.stop_name = stop.at("name").AsString();
                new_stop.pos.lat = stop.at("latitude").AsDouble();
                new_stop.pos.lng = stop.at("longitude").AsDouble();

                catalogue.AddStop(std::move(new_stop));
                if (stop.count("road_distances")) {
                    for (const auto &[stop_to, distance]: stop.at("road_distances").AsMap()) {
                        catalogue.AddDistanceBetweenStops(stop_name, stop_to, distance.AsInt());
                    }
                }
            }

            void JSON_Reader::ExecuteBusRequest(const Node &bus_node,
                                                TransportCatalogue &catalogue) {
                const auto &bus = bus_node.AsMap();
                Bus new_bus;
                new_bus.bus_name = bus.at("name").AsString();

                for (const auto &stop: bus.at("stops").AsArray()) {
                    new_bus.stops_on_route.emplace_back(stop.AsString());
                }

                new_bus.is_circle_route = bus.at("is_roundtrip").AsBool();

                if (!new_bus.is_circle_route) {
                    int next_stop_from_end = new_bus.stops_on_route.size() - 2;
                    for (int i = next_stop_from_end; i >= 0; i--) {
                        new_bus.stops_on_route.emplace_back(new_bus.stops_on_route[i]);
                    }
                }
                catalogue.AddBus(std::move(new_bus));
            }

            Document JSON_Reader::ExecuteStatRequests(RequestHandler &handler) {
                Array result;
                std::string_view request_type;
                for (const auto &request: stat_requests_.AsArray()) {
                    request_type = request.AsMap().at("type").AsString();

                    if (request_type == "Bus") {
                        result.emplace_back(MakeBusResponse(request, handler));
                    } else if (request_type == "Stop") {
                        result.emplace_back(MakeStopResponse(request, handler));
                    } else if (request_type == "Map") {
                        result.emplace_back(MakeMapResponse(request, handler));
                    } else if(request_type == "Route") {
                        result.emplace_back(MakeRouteResponse(request, handler));
                    }
                }
                return Document{result};
            }

            map_renderer::RenderSettings JSON_Reader::GetRenderSettings() {
                map_renderer::RenderSettings result;
                const auto &settings = render_settings_.AsMap();
                result.width_ = settings.at("width").AsDouble();
                result.height_ = settings.at("height").AsDouble();
                result.padding_ = settings.at("padding").AsDouble();
                result.line_width_ = settings.at("line_width").AsDouble();
                result.bus_label_font_size_ = settings.at("bus_label_font_size").AsInt();
                result.bus_label_offset_.first = settings.at("bus_label_offset").AsArray()[0].AsDouble();
                result.bus_label_offset_.second = settings.at("bus_label_offset").AsArray()[1].AsDouble();
                result.stop_label_font_size_ = settings.at("stop_label_font_size").AsInt();
                result.stop_label_offset_.first = settings.at("stop_label_offset").AsArray()[0].AsDouble();
                result.stop_label_offset_.second = settings.at("stop_label_offset").AsArray()[1].AsDouble();
                result.stop_radius_ = settings.at("stop_radius").AsDouble();
                result.underlayer_width_ = settings.at("underlayer_width").AsDouble();
                result.underlayer_color_ = ParseColor(settings.at("underlayer_color"));
                for (const auto &color: settings.at("color_palette").AsArray()) {
                    result.color_palette_.emplace_back(ParseColor(color));
                }
                return result;
            }

            svg::Color JSON_Reader::ParseColor(const Node &color_node) {
                if (color_node.IsString()) {
                    return color_node.AsString();
                } else if (color_node.IsArray()) {
                    const auto &color = color_node.AsArray();
                    if (color.size() == 3) {
                        return svg::Rgb{static_cast<uint8_t>(color[0].AsInt()),
                                        static_cast<uint8_t>(color[1].AsInt()),
                                        static_cast<uint8_t>(color[2].AsInt())};
                    } else if (color.size() == 4) {
                        return svg::Rgba{static_cast<uint8_t>(color[0].AsInt()),
                                         static_cast<uint8_t>(color[1].AsInt()),
                                         static_cast<uint8_t>(color[2].AsInt()),
                                         color[3].AsDouble()};
                    }
                }
                return svg::NoneColor;
            }

            Node JSON_Reader::MakeStopResponse(const Node &stop_node,
                                               RequestHandler &handler) {
                Builder builder;
                const auto &stop = handler.GetStopInfo(stop_node.AsMap().at("name").AsString());
                if (!stop) {
                    builder.StartDict()
                        .Key("request_id").Value(stop_node.AsMap().at("id"))
                        .Key("error_message").Value(std::string("not found"))
                        .EndDict();
                    return builder.Build();
                } else {
                    Array buses;
                    for (const auto &bus: stop->buses_on_stop) {
                        buses.emplace_back(bus);
                    }
                    builder.StartDict()
                        .Key("request_id").Value(stop_node.AsMap().at("id"))
                        .Key("buses").Value(std::move(buses))
                        .EndDict();
                    return builder.Build();
                }
            }

            Node JSON_Reader::MakeBusResponse(const Node &bus_node, RequestHandler &handler) {
                Builder builder;
                const auto &bus = handler.GetBusInfo(bus_node.AsMap().at("name").AsString());
                if (!bus) {
                    builder.StartDict()
                        .Key("request_id").Value(bus_node.AsMap().at("id"))
                        .Key("error_message").Value(std::string("not found"))
                        .EndDict();
                    return builder.Build();
                } else {
                    double geo_length = 0.0;
                    int route_length = 0;
                    const auto &stops = bus->stops_on_route;
                    for (size_t i = 0; i <= stops.size() - 2; i++) {
                        handler.CalcDistance(geo_length, route_length, stops[i], stops[i + 1]);
                    }
                    builder.StartDict()
                        .Key("request_id").Value(bus_node.AsMap().at("id"))
                        .Key("route_length").Value(route_length)
                        .Key("curvature").Value((route_length * 1.0) / geo_length)
                        .Key("stop_count").Value(int(stops.size()))
                        .Key("unique_stop_count").Value(int(handler.CountUniqueStops(bus)))
                        .EndDict();
                    return builder.Build();
                }
            }

            Node JSON_Reader::MakeMapResponse(const Node &map_node, RequestHandler &handler) {
                Builder builder;

                map_renderer::RenderSettings settings = GetRenderSettings();
                map_renderer::MapRenderer renderer(settings);
                svg::Document map = renderer.RenderMap(handler.GetDataForRender());

                std::ostringstream str_map;
                map.Render(str_map);
                builder.StartDict()
                    .Key("request_id").Value(map_node.AsMap().at("id"))
                    .Key("map").Value(str_map.str())
                    .EndDict();
                return builder.Build();
            }
            Node JSON_Reader::MakeRouteResponse(const Node& route_node, RequestHandler& handler){
                Builder builder;
                RouteSettings settings;

                settings.wait_time = routing_settings_.AsMap().at("bus_wait_time").AsDouble();
                settings.velocity = routing_settings_.AsMap().at("bus_velocity").AsDouble();
                struct EdgeInfoGetter {
                    Node operator()(const StopEdge& edge_info) {
                        using namespace std::literals;

                        return Builder{}.StartDict()
                                .Key("type").Value("Wait"s)
                                .Key("stop_name").Value(std::string(edge_info.name))
                                .Key("time").Value(edge_info.time)
                                .EndDict()
                                .Build();
                    }

                    Node operator()(const BusEdge& edge_info) {
                        using namespace std::literals;

                        return Builder{}.StartDict()
                                .Key("type").Value("Bus"s)
                                .Key("bus").Value(std::string(edge_info.bus_name))
                                .Key("span_count").Value(static_cast<int>(edge_info.span_count))
                                .Key("time").Value(edge_info.time)
                                .EndDict()
                                .Build();
                    }
                };

                const auto& from = route_node.AsMap().at("from").AsString();
                const auto& to = route_node.AsMap().at("to").AsString();
                const auto& id = route_node.AsMap().at("id").AsInt();

                const auto& route_info = handler.GetRouteInformation(from, to, settings);

                if(!route_info){
                    builder.StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value(std::string ("not found"))
                    .EndDict();
                    return builder.Build();
                }else{
                    Array items;
                    for(const auto& item : route_info->edges){
                        items.emplace_back(std::visit(EdgeInfoGetter{}, item));
                    }

                    builder.StartDict()
                    .Key("request_id").Value(id)
                    .Key("total_time").Value(route_info->total_time)
                    .Key("items").Value(items)
                    .EndDict();
                    return builder.Build();
                }
            }

        }
    }
}