#include "map_renderer.h"
namespace map_renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(catalogue::geoposition::Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    SphereProjector MapRenderer::Get_projector(std::vector<catalogue::geoposition::Coordinates> stop_coordinates){
        return SphereProjector{stop_coordinates.begin(),
                               stop_coordinates.end(),
                               render_settings_.width_,
                               render_settings_.height_,
                               render_settings_.padding_};
    }

    void MapRenderer::Set_line_properties(svg::Polyline& line, int line_num){
        line.SetFillColor("none");
        line.SetStrokeColor(Get_color_for_this_line(line_num));
        line.SetStrokeWidth(render_settings_.line_width_);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    svg::Color MapRenderer::Get_color_for_this_line(int line_num){
        auto palette_size = render_settings_.color_palette_.size();
        auto index = line_num % palette_size;
        return render_settings_.color_palette_[index];
    }

    svg::Document MapRenderer::Render_map(const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>>& data) {
        svg::Document map;
        std::vector<catalogue::geoposition::Coordinates> coordinates;
        std::set<domain::Stop*, CompareStopPointers> sorted_stops;

        for(const auto& [bus, stops] : data){
            for(const auto& stop : stops){
                coordinates.push_back(stop->pos);
                sorted_stops.insert(stop);
            }
        }
        SphereProjector projector = Get_projector(coordinates);
        Add_lines(projector, data, map);
        Add_route_labels(projector, data, map);
        Add_stop_circles(projector, sorted_stops, map);
        Add_stop_labels(projector, sorted_stops, map);
        return map;
    }

    void MapRenderer::Add_lines(map_renderer::SphereProjector &projector,
                                const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>> &data,
                                svg::Document& doc) {
        int color_count = 0;
        for(const auto& [bus, stops] : data){
            if(!stops.empty()){
                svg::Polyline line;
                for(const auto& stop : stops){
                    line.AddPoint(projector(stop->pos));
                }
                //нужно ли определение настроек линии вынести в отдельную функцию?
                line.SetFillColor("none");
                line.SetStrokeColor(Get_color_for_this_line(color_count));
                line.SetStrokeWidth(render_settings_.line_width_);
                line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                doc.Add(std::move(line));
                color_count++;
            }
        }
    }

    void MapRenderer::Add_route_labels(map_renderer::SphereProjector &projector,
                                       const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>> &data,
                                       svg::Document &doc) {
        int color_count = 0;
        for(const auto& [bus, stops] : data){
            if(!stops.empty()){
                //нужно ли определение настроек текста вынести в отдельную функцию?
                svg::Text route_label;
                route_label.SetData(bus->bus_name);
                route_label.SetOffset(svg::Point{render_settings_.bus_label_offset_});
                route_label.SetFontWeight("bold");
                route_label.SetFontFamily("Verdana");
                route_label.SetFontSize(render_settings_.bus_label_font_size_);

                svg::Text underlayer = route_label;
                underlayer.SetFillColor(render_settings_.underlayer_color_);
                underlayer.SetStrokeColor(render_settings_.underlayer_color_);
                underlayer.SetStrokeWidth(render_settings_.underlayer_width_);
                underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

                route_label.SetFillColor(Get_color_for_this_line(color_count));

                const auto& start_stop = stops[0];
                const auto& final_stop = stops[stops.size()/2];

                route_label.SetPosition(projector(start_stop->pos));
                underlayer.SetPosition(projector(start_stop->pos));

                if(!bus->is_circle_route && (start_stop->stop_name != final_stop->stop_name)){
                    svg::Text final_underlayer = underlayer;
                    svg::Text final_route_label = route_label;
                    final_underlayer.SetPosition(projector(final_stop->pos));
                    final_route_label.SetPosition(projector(final_stop->pos));
                    doc.Add(underlayer);
                    doc.Add(route_label);
                    doc.Add(final_underlayer);
                    doc.Add(final_route_label);
                    color_count++;
                    continue;
                }

                doc.Add(underlayer);
                doc.Add(route_label);
                color_count++;
            }
        }

    }

    void MapRenderer::Add_stop_circles(map_renderer::SphereProjector &projector,
                                       const std::set<domain::Stop*, CompareStopPointers>& data,
                                       svg::Document &doc) {

        for(const auto& stop : data){
            if(!stop->buses_on_stop.empty()){
                svg::Circle stop_circle;
                stop_circle.SetCenter(projector(stop->pos));
                stop_circle.SetRadius(render_settings_.stop_radius_);
                stop_circle.SetFillColor("white");
                doc.Add(stop_circle);
            }
        }
    }

    void MapRenderer::Add_stop_labels(map_renderer::SphereProjector &projector,
                                      const std::set<domain::Stop*, CompareStopPointers>& data,
                                      svg::Document &doc) {
        svg::Text underlayer;
        underlayer.SetOffset(render_settings_.stop_label_offset_);
        underlayer.SetFontSize(render_settings_.stop_label_font_size_);
        underlayer.SetFontFamily("Verdana");

        svg::Text stop_label = underlayer;
        stop_label.SetFillColor("black");

        underlayer.SetFillColor(render_settings_.underlayer_color_);
        underlayer.SetStrokeColor(render_settings_.underlayer_color_);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width_);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for(const auto& stop : data){
            if(!stop->buses_on_stop.empty()){
                underlayer.SetPosition(projector(stop->pos));
                underlayer.SetData(stop->stop_name);
                stop_label.SetPosition(projector(stop->pos));
                stop_label.SetData(stop->stop_name);
                doc.Add(underlayer);
                doc.Add(stop_label);
            }
        }
    }

}
